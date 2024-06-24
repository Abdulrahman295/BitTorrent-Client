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
// #include <winsock2.h>
// #include <ws2tcpip.h>

#include <cpr/cpr.h>

#include "client/client.hpp"
#include "bencode/decode.hpp"

using namespace std::string_literals;

// #pragma comment(lib, "Ws2_32.lib")

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

    return this->parse_server_response(r.text);
}

std::string Client::parse_server_response(std::string response)
{
    Decode decode = Decode();
    auto decoded_response = decode.decode_bencoded_value(response);
    std::string peers_str = decoded_response["peers"].get<std::string>();

    std::string result = ""; // represents the the peers IP addresses

    size_t number_of_peers = peers_str.size() / 6;
    for (size_t i = 0; i < number_of_peers; i++)
    {
        std::string ip = this->parse_ip(peers_str, i);
        std::string port = this->parse_port(peers_str, i);

        result += ip + ":" + port + "\n";
    }

    return result;
}

std::string Client::parse_ip(std::string peers_str, size_t index)
{
    std::string ip_raw = peers_str.substr(index * 6, 4);
    std::string ip = std::to_string(static_cast<unsigned char>(ip_raw[0])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[1])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[2])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[3]));

    return ip;
}

std::string Client::parse_port(std::string peers_str, size_t index)
{
    std::string port_raw = peers_str.substr(index * 6 + 4, 2);
    uint16_t port =
        (static_cast<uint16_t>(static_cast<unsigned char>(port_raw[0]) << 8)) |
        static_cast<uint16_t>(static_cast<unsigned char>(port_raw[1]));

    return std::to_string(port);
}

void Client::create_connection(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    // Initialize Winsock
    // WSADATA wsaData;
    // int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    // if (iResult != 0)
    // {
    //     throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
    // }

    // Create a TCP socket
    // SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // if (ConnectSocket == INVALID_SOCKET)
    // {
    //     WSACleanup();
    //     throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
    // }

    // Resolve the peer address and port
    // sockaddr_in peerAddr;
    // peerAddr.sin_family = AF_INET;
    // peerAddr.sin_port = htons(std::stoi(peer_port));
    // inet_pton(AF_INET, peer_ip.c_str(), &peerAddr.sin_addr);

    // Connect
    // iResult = connect(ConnectSocket, (SOCKADDR *)&peerAddr, sizeof(peerAddr));
    // if (iResult == SOCKET_ERROR)
    // {
    //     closesocket(ConnectSocket);
    //     WSACleanup();
    //     throw std::runtime_error("connect failed: " + std::to_string(WSAGetLastError()));
    // }

    // this->sock = ConnectSocket;

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

std::string Client::create_handshake_message(MetaInfo metaInfo)
{
    std::string handshake_message = "\x13"s                               // length of the protocol string
                                    + "BitTorrent protocol"s              // protocol string
                                    + "\x00\x00\x00\x00\x00\x00\x00\x00"s // reserved
                                    + metaInfo.get_info_string()          // info hash
                                    + "00112233445566778899"s;            // peer id

    return handshake_message;
}

void Client::send_message(std::string message)
{
    // int iResult = send(this->sock, message.c_str(), message.size(), 0);
    // if (iResult == SOCKET_ERROR)
    // {
    //     closesocket(this->sock);
    //     WSACleanup();
    //     throw std::runtime_error("send failed: " + std::to_string(WSAGetLastError()));
    // }

    ssize_t iResult = send(this->sock, message.c_str(), message.size(), 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("send failed");
    }
}

std::string Client::receive_message()
{
    // char buffer[1024];
    // int iResult = recv(this->sock, buffer, 1024, 0);
    // if (iResult == SOCKET_ERROR)
    // {
    //     closesocket(this->sock);
    //     WSACleanup();
    //     throw std::runtime_error("recv failed: " + std::to_string(WSAGetLastError()));
    // }

    // return std::string(buffer, iResult);

    char buffer[1024];
    ssize_t iResult = recv(this->sock, buffer, 1024, 0);
    if (iResult < 0)
    {
        close(this->sock);
        throw std::runtime_error("recv failed");
    }

    return std::string(buffer, iResult);
}

std::string Client::parse_peer_response(std::string response)
{
    size_t start_pos =
        28    // length of the protocol string + reserved bytes
        + 20; // length of the info hash

    size_t peer_id_length = 20;

    std::string peer_id_raw = response.substr(start_pos, peer_id_length);

    // get peer_id in hex format
    std::stringstream ss;
    for (unsigned char c : peer_id_raw)
    {
        ss << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<int>(c);
    }

    return ss.str();
}

std::string Client::get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    this->create_connection(metaInfo, peer_ip, peer_port);

    std::string handshake_message = this->create_handshake_message(metaInfo);

    this->send_message(handshake_message);

    std::string response = this->receive_message();

    std::string peer_id = this->parse_peer_response(response);

    // closesocket(this->sock);
    // WSACleanup();

    close(this->sock);

    return peer_id;
}