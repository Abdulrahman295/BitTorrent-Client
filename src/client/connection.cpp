#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

#include "client/connection.hpp"
#include "metainfo/metainfo.hpp"
#include "messageHandler/message.hpp"
#include "messageHandler/messageHandler.hpp"

Connection::Connection(const std::string &peer_ip, const std::string &peer_port)
{

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

Connection::~Connection()
{
    if (this->sock != 0)
    {
        close(this->sock);
    }
}

void Connection::send_message(std::vector<uint8_t> message)
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

std::string Connection::receive_handshake_message()
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

Message Connection::receive_peer_message()
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

std::vector<uint8_t> Connection::fetch_piece_blocks(MetaInfo metaInfo, size_t piece_index)
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