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

#include <cpr/cpr.h>

#include "client/client.hpp"
#include "bencode/decode.hpp"
#include "messageHandler/messageHandler.hpp"

using namespace std::string_literals;

Client::Client()
{
    this->sock = 0;
}

std::string Client::discover_peers(MetaInfo metaInfo)
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

void Client::send_message(std::string message)
{
    ssize_t iResult = send(this->sock, message.c_str(), message.size(), 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("send failed");
    }
}

std::string Client::receive_message()
{
    char buffer[1024];
    ssize_t iResult = recv(this->sock, buffer, 1024, 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("recv failed");
    }

    return std::string(buffer, iResult);
}

std::string Client::get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    this->create_connection(metaInfo, peer_ip, peer_port);

    std::string handshake_message = MessageHandler::create_handshake_message(metaInfo);

    this->send_message(handshake_message);

    std::string response = this->receive_message();

    std::string peer_id = MessageHandler::parse_handshake_response(response);

    close(this->sock);

    return peer_id;
}