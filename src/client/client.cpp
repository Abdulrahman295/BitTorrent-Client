#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <algorithm>
#include <queue>
#include <mutex>
#include <thread>

#include <cpr/cpr.h>

#include "client/client.hpp"
#include "bencode/decode.hpp"
#include "messageHandler/messageHandler.hpp"
#include "messageHandler/message.hpp"
#include "metainfo/sha1.hpp"
#include "client/connection.hpp"

using namespace std::string_literals;

std::vector<std::string> Client::discover_peers(MetaInfo metaInfo)
{

    cpr::Response r = cpr::Get(cpr::Url{metaInfo.get_announceURL()},
                               cpr::Parameters{
                                   {"peer_id", "00112233445566778899"},
                                   {"port", "6881"},
                                   {"uploaded", "0"},
                                   {"downloaded", "0"},
                                   {"left", std::to_string(metaInfo.get_file_size())},
                                   {"compact", "1"},
                                   {"info_hash", metaInfo.get_info_string()}});

    return MessageHandler::parse_server_response(r.text);
}

std::string Client::get_peer_id(MetaInfo metaInfo, Connection &peerConnection)
{
    std::vector<uint8_t> handshake_message = MessageHandler::create_handshake_message(metaInfo);

    peerConnection.send_message(handshake_message);

    std::string response = peerConnection.receive_handshake_message();

    std::string peer_id = MessageHandler::parse_handshake_response(response);

    return peer_id;
}

Connection Client::connect_to_peer(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    Connection peerConnection(peer_ip, peer_port);
    std::string peerID = this->get_peer_id(metaInfo, peerConnection); // Handshake with the peer

    // Wait for a bitfield message from the peer indicating which pieces it has
    Message bitfield_response = peerConnection.receive_peer_message();
    if (bitfield_response.get_type() != MessageType::BITFIELD)
    {
        throw std::runtime_error("Expected BITFIELD(ID=5) message, but received message of ID: " + std::to_string(static_cast<int>(bitfield_response.get_type())));
    }

    // send interested message and wait for unchoke message
    std::vector<uint8_t> interested_message = MessageHandler::create_interested_message();
    peerConnection.send_message(interested_message);

    Message unchoke_response = peerConnection.receive_peer_message();
    if (unchoke_response.get_type() != MessageType::UNCHOKE)
    {
        throw std::runtime_error("Expected UNCHOKE message(ID=1), but received  message of ID: " + std::to_string(static_cast<int>(unchoke_response.get_type())));
    }

    return peerConnection;
}

void Client::verify_piece(MetaInfo metaInfo, std::vector<uint8_t> piece_data, size_t piece_index)
{
    auto sha = SHA1();

    sha.update(std::string_view(reinterpret_cast<const char *>(piece_data.data()), piece_data.size()));

    const auto calculated_piece_hash = sha.final();

    const auto expected_piece_hash = metaInfo.get_pieces_hash()[piece_index];

    if (calculated_piece_hash != expected_piece_hash)
    {
        throw std::runtime_error("Piece hash verification failed, expected: " + expected_piece_hash + " but got: " + calculated_piece_hash);
    }
}

void Client::save_to_file(std::string output_file, std::vector<uint8_t> data)
{
    std::ofstream file(output_file, std::ios::binary);
    file.write(reinterpret_cast<char const *>(data.data()), data.size());
    file.close();
}

void Client::download_piece(MetaInfo metaInfo, std::string output_file, size_t piece_index)
{
    std::vector<std::string> peers = this->discover_peers(metaInfo);
    if (peers.size() == 0)
        throw std::runtime_error("No peers found");

    std::string peer_ip = peers[0].substr(0, peers[0].find(":"));
    std::string peer_port = peers[0].substr(peers[0].find(":") + 1);

    Connection peerConnection = connect_to_peer(metaInfo, peer_ip, peer_port);

    // send a request message Wait for a piece message for each block
    std::vector<uint8_t> piece_data = peerConnection.fetch_piece_blocks(metaInfo, piece_index);

    // verify the piece hash before saving it to the output file
    this->verify_piece(metaInfo, piece_data, piece_index);

    // save the data to the output file
    this->save_to_file(output_file, piece_data);
}

void Client::worker(MetaInfo metaInfo, const std::string peer_ip, const std::string peer_port)
{
    try
    {
        Connection peerConnection = this->connect_to_peer(metaInfo, peer_ip, peer_port);

        while (true)
        {
            size_t piece_index;
            {
                // get the next piece index from the work queue, if the queue is empty, then all pieces have been downloaded and the worker can exit
                std::lock_guard<std::mutex> lock(work_queue_mutex);
                if (work_queue.empty())
                {
                    break;
                }
                piece_index = work_queue.front();
                work_queue.pop();
            }

            try
            {
                std::vector<uint8_t> piece_data = peerConnection.fetch_piece_blocks(metaInfo, piece_index);
                this->verify_piece(metaInfo, piece_data, piece_index);

                {
                    std::lock_guard<std::mutex> lock(downloaded_pieces_mutex);
                    downloaded_pieces[piece_index] = std::move(piece_data);
                }
            }
            catch (const std::exception &e)
            {
                // If the piece download fails, add it back to the work queue
                std::cerr << "Failed to download piece " << piece_index << ": " << e.what() << std::endl;
                std::lock_guard<std::mutex> lock(work_queue_mutex);
                work_queue.push(piece_index);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Worker failed: " << e.what() << std::endl;
    }
}

void Client::download_file(MetaInfo metaInfo, std::string output_file)
{
    std::vector<std::string> peers = this->discover_peers(metaInfo);
    if (peers.empty())
    {
        throw std::runtime_error("No peers found");
    }

    for (size_t i = 0; i < metaInfo.get_pieces_hash().size(); ++i)
    {
        work_queue.push(i);
    }
    downloaded_pieces.resize(metaInfo.get_pieces_hash().size());

    const int num_threads = std::min(static_cast<int>(peers.size()), 5); // Limit to 5 threads or number of peers
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i)
    {
        std::string peer_ip = peers[i].substr(0, peers[i].find(":"));
        std::string peer_port = peers[i].substr(peers[i].find(":") + 1);
        threads.emplace_back(&Client::worker, this, metaInfo, peer_ip, peer_port); // Start a worker thread
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    // Concatenate all the pieces into a single data vector
    std::vector<uint8_t> data;
    for (const auto &piece : downloaded_pieces)
    {
        data.insert(data.end(), piece.begin(), piece.end());
    }

    // Save the data to the output file
    save_to_file(output_file, data);
}
