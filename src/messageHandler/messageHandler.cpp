#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>

#include "messageHandler/messageHandler.hpp"
#include "metainfo/metainfo.hpp"
#include "bencode/decode.hpp"

using namespace std::string_literals;

std::string MessageHandler::parse_server_response(std::string response)
{
    Decode decode = Decode();
    auto decoded_response = decode.decode_bencoded_value(response);
    std::string peers_str = decoded_response["peers"].get<std::string>();

    std::string result = ""; // represents the the peers IP addresses

    size_t number_of_peers = peers_str.size() / 6;
    for (size_t i = 0; i < number_of_peers; i++)
    {
        std::string ip = MessageHandler::parse_ip(peers_str, i);
        std::string port = MessageHandler::parse_port(peers_str, i);

        result += ip + ":" + port + "\n";
    }

    return result;
}

std::string MessageHandler::parse_ip(std::string peers_str, size_t index)
{
    std::string ip_raw = peers_str.substr(index * 6, 4);
    std::string ip = std::to_string(static_cast<unsigned char>(ip_raw[0])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[1])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[2])) + "." +
                     std::to_string(static_cast<unsigned char>(ip_raw[3]));

    return ip;
}

std::string MessageHandler::parse_port(std::string peers_str, size_t index)
{
    std::string port_raw = peers_str.substr(index * 6 + 4, 2);
    uint16_t port =
        (static_cast<uint16_t>(static_cast<unsigned char>(port_raw[0]) << 8)) |
        static_cast<uint16_t>(static_cast<unsigned char>(port_raw[1]));

    return std::to_string(port);
}

std::string MessageHandler::parse_handshake_response(std::string response)
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

std::string MessageHandler::create_handshake_message(MetaInfo metaInfo)
{
    std::string handshake_message = "\x13"s                               // length of the protocol string
                                    + "BitTorrent protocol"s              // protocol string
                                    + "\x00\x00\x00\x00\x00\x00\x00\x00"s // reserved
                                    + metaInfo.get_info_string()          // info hash
                                    + "00112233445566778899"s;            // peer id

    return handshake_message;
}
