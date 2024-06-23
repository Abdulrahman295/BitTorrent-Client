#include <iostream>
#include <string>

#include <cpr/cpr.h>

#include "client/client.hpp"
#include "bencode/decode.hpp"

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

    return this->parse_tracker_response(r.text);
}

/**
 * @brief parse the info hash from a hex string
 *
 * @param hex
 * @return std::string
 */

/**
 * @brief parse the tracker response and return the peers IP addresses
 *
 * @param response
 * @return std::string
 */
std::string Client::parse_tracker_response(std::string response)
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