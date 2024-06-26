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

#include <cpr/cpr.h>

#include "client/client.hpp"
#include "bencode/decode.hpp"
#include "messageHandler/messageHandler.hpp"
#include "messageHandler/message.hpp"
#include "metainfo/sha1.hpp"

using namespace std::string_literals;

Client::Client()
{
    this->sock = 0;
}

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

void Client::create_connection(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    if (this->sock != 0)
    {
        close(this->sock);
    }

    // Create a TCP socket
    int ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ConnectSocket < 0)
    {
        throw std::runtime_error("socket failed");
    }

    // Resolve the peer address and port
    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(std::stoi(peer_port));
    inet_pton(AF_INET, peer_ip.c_str(), &peerAddr.sin_addr);

    // Connect
    ssize_t iResult = connect(ConnectSocket, (struct sockaddr *)&peerAddr, sizeof(peerAddr));
    if (iResult < 0)
    {
        close(ConnectSocket);
        throw std::runtime_error("connect failed");
    }

    this->sock = ConnectSocket;
}

void Client::send_message(std::vector<uint8_t> message)
{
    if (this->sock == 0)
    {
        throw std::runtime_error("Socket not connected");
    }

    ssize_t iResult = send(this->sock, message.data(), message.size(), 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("send failed");
    }
}

std::string Client::receive_handshake_message()
{
    if (this->sock == 0)
    {
        throw std::runtime_error("Socket not connected");
    }

    char buffer[1024];
    ssize_t iResult = recv(this->sock, buffer, 1024, 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("recv failed");
    }

    return std::string(buffer, iResult);
}

Message Client::receive_peer_message()
{
    if (this->sock == 0)
    {
        throw std::runtime_error("Socket not connected");
    }

    uint32_t messageLength;
    ssize_t iResult = recv(this->sock, &messageLength, sizeof(messageLength), 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("recv failed");
    }

    messageLength = ntohl(messageLength);

    uint8_t messageID;
    iResult = recv(this->sock, &messageID, sizeof(messageID), 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("recv failed");
    }

    // payload size (total message length - 1 byte for ID part)
    size_t payloadSize = messageLength - 1;

    // Receive the payload
    std::vector<uint8_t> payload(payloadSize);
    size_t totalBytesRead = 0;
    while (totalBytesRead < payloadSize)
    {
        ssize_t bytesRead = recv(this->sock, payload.data() + totalBytesRead, payloadSize - totalBytesRead, 0);
        if (bytesRead <= 0)
        {
            throw std::runtime_error("Failed to receive complete payload");
        }
        totalBytesRead += bytesRead;
    }

    return Message(static_cast<MessageType>(messageID), messageLength, payload);
}

std::string Client::get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    this->create_connection(metaInfo, peer_ip, peer_port);

    std::vector<uint8_t> handshake_message = MessageHandler::create_handshake_message(metaInfo);

    this->send_message(handshake_message);

    std::string response = this->receive_handshake_message();

    std::string peer_id = MessageHandler::parse_handshake_response(response);

    return peer_id;
}

std::vector<uint8_t> Client::fetch_piece_blocks(MetaInfo metaInfo, size_t piece_index)
{
    const uint32_t BLOCK_SIZE = 16 * 1024;
    uint32_t block_index = 0;
    uint32_t block_offset = 0;
    uint32_t current_block_length = BLOCK_SIZE;

    std::vector<std::string> pieces_hash = metaInfo.get_pieces_hash();
    uint32_t piece_length;
    if (piece_index >= pieces_hash.size())
    {
        throw std::runtime_error("Invalid piece index");
    }
    else if (piece_index == pieces_hash.size() - 1)
    {

        piece_length = metaInfo.get_file_size() % metaInfo.get_piece_length();
    }
    else
    {
        piece_length = metaInfo.get_piece_length();
    }

    std::vector<uint8_t> piece_data(piece_length);

    while (block_index * BLOCK_SIZE < piece_length)
    {
        // adjust block length for the last block
        if (block_offset + BLOCK_SIZE > piece_length)
        {
            current_block_length = piece_length - block_offset;
        }

        std::vector<uint8_t> request_message = MessageHandler::create_request_message(piece_index, block_offset, current_block_length);
        this->send_message(request_message);

        Message piece_response = this->receive_peer_message();

        if (piece_response.get_type() == MessageType::CHOKE)
        {
            break;
        }

        if (piece_response.get_type() != MessageType::PIECE)
        {
            throw std::runtime_error("Expected PIECE message(ID=7), but received message of ID: " + std::to_string(static_cast<int>(piece_response.get_type())) + " at block index: " + std::to_string(block_index) + " and block offset: " + std::to_string(block_offset) + " with length: " + std::to_string(current_block_length));
        }

        Block block = piece_response.get_block();
        std::copy(block.data.begin(), block.data.end(), piece_data.begin() + block.begin);

        block_index++;
        block_offset += BLOCK_SIZE;
    }

    return piece_data;
}

void Client::save_to_file(std::string output_file, std::vector<uint8_t> data)
{
    std::ofstream file(output_file, std::ios::binary);
    file.write(reinterpret_cast<char const *>(data.data()), data.size());
    file.close();
}

void Client::download_piece(MetaInfo metaInfo, std::string output_file, size_t piece_index)
{
    // discover peers and make a handeshake with one of them
    std::vector<std::string> peers = this->discover_peers(metaInfo);

    if (peers.size() == 0)
        throw std::runtime_error("No peers found");

    std::string peer_ip = peers[0].substr(0, peers[0].find(":"));
    std::string peer_port = peers[0].substr(peers[0].find(":") + 1);
    std::string peerID = this->get_peer_id(metaInfo, peer_ip, peer_port); // establish connection and get peer id

    // Wait for a bitfield message from the peer indicating which pieces it has
    Message bitfield_response = this->receive_peer_message();
    if (bitfield_response.get_type() != MessageType::BITFIELD)
    {
        throw std::runtime_error("Expected BITFIELD(ID=5) message, but received message of ID: " + std::to_string(static_cast<int>(bitfield_response.get_type())));
    }

    // send interested message and wait for unchoke message
    std::vector<uint8_t> interested_message = MessageHandler::create_interested_message();
    this->send_message(interested_message);

    Message unchoke_response = this->receive_peer_message();
    if (unchoke_response.get_type() != MessageType::UNCHOKE)
    {
        throw std::runtime_error("Expected UNCHOKE message(ID=1), but received  message of ID: " + std::to_string(static_cast<int>(unchoke_response.get_type())));
    }

    // send a request message Wait for a piece message for each block
    std::vector<uint8_t> piece_data = this->fetch_piece_blocks(metaInfo, piece_index);

    // save the data to the output file
    this->save_to_file(output_file, piece_data);
}

void Client::close_connection()
{
    if (this->sock == 0)
        return;

    ssize_t iResult = close(this->sock);
    if (iResult < 0)
    {
        throw std::runtime_error("close failed");
    }
}