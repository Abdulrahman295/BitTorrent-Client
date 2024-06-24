#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <cpr/cpr.h>

#include "client/client.hpp"
#include "bencode/decode.hpp"

using namespace std::string_literals;

#pragma comment(lib, "Ws2_32.lib")

/**
 * @brief sends a get request to the tracker server to discover peers IP addresses
 *
 * @return std::string
 */
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

/**
 * @brief parse the tracker response and return the peers IP addresses
 *
 * @param response
 * @return std::string
 */
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

/**
 * @brief parse the IP address from the peers string
 *
 * @param peers_str
 * @param index
 * @return std::string
 */
std::string Client::parse_ip(std::string peers_str, size_t index)
{
    std::string ip_raw = peers_str.substr(index * 6, 4);
    std::string ip = std::to_string(static_cast<unsigned char>(ip_raw[0])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[1])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[2])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[3]));

    return ip;
}

/**
 * @brief parse the port from the peers string
 *
 * @param peers_str
 * @param index
 * @return std::string
 */
std::string Client::parse_port(std::string peers_str, size_t index)
{
    std::string port_raw = peers_str.substr(index * 6 + 4, 2);
    uint16_t port =
        (static_cast<uint16_t>(static_cast<unsigned char>(port_raw[0]) << 8)) |
        static_cast<uint16_t>(static_cast<unsigned char>(port_raw[1]));

    return std::to_string(port);
}

/**
 * @brief creates a TCP connection with a peer
 *
 * @param metaInfo
 * @param peer_ip
 * @param peer_port
 * @return std::string
 */
void Client::create_connection(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
    }

    // Create a TCP socket
    SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET)
    {
        WSACleanup();
        throw std::runtime_error("socket failed: " + std::to_string(WSAGetLastError()));
    }

    // Resolve the peer address and port
    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(std::stoi(peer_port));
    inet_pton(AF_INET, peer_ip.c_str(), &peerAddr.sin_addr);

    // Connect
    iResult = connect(ConnectSocket, (SOCKADDR *)&peerAddr, sizeof(peerAddr));
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        WSACleanup();
        throw std::runtime_error("connect failed: " + std::to_string(WSAGetLastError()));
    }

    this->sock = ConnectSocket;
}

/**
 * @brief creates a handshake message
 *
 * @param metaInfo
 * @return std::string
 */
std::string Client::create_handshake_message(MetaInfo metaInfo)
{
    std::string handshake_message = "\x13"s                               // length of the protocol string
                                    + "BitTorrent protocol"s              // protocol string
                                    + "\x00\x00\x00\x00\x00\x00\x00\x00"s // reserved
                                    + metaInfo.get_info_string()          // info hash
                                    + "00112233445566778899"s;            // peer id

    return handshake_message;
}

/**
 * @brief sends a message to the peer
 *
 * @param message
 */
void Client::send_message(std::string message)
{
    int iResult = send(this->sock, message.c_str(), message.size(), 0);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(this->sock);
        WSACleanup();
        throw std::runtime_error("send failed: " + std::to_string(WSAGetLastError()));
    }
}

/**
 * @brief receives a message from the peer
 *
 * @return std::string
 */
std::string Client::receive_message()
{
    char buffer[1024];
    int iResult = recv(this->sock, buffer, 1024, 0);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(this->sock);
        WSACleanup();
        throw std::runtime_error("recv failed: " + std::to_string(WSAGetLastError()));
    }

    return std::string(buffer, iResult);
}

/**
 * @brief parse the peer response and return the peer id
 *
 * @param response
 * @return std::string
 */
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

/**
 * @brief returns the peer id
 *
 * @return std::string
 */
std::string Client::get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port)
{
    this->create_connection(metaInfo, peer_ip, peer_port);

    std::string handshake_message = this->create_handshake_message(metaInfo);

    this->send_message(handshake_message);

    std::string response = this->receive_message();

    std::string peer_id = this->parse_peer_response(response);

    closesocket(this->sock);
    WSACleanup();

    return peer_id;
}