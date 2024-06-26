#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdint>
#include <vector>
#include <arpa/inet.h>

#include "messageHandler/messageHandler.hpp"
#include "messageHandler/message.hpp"
#include "metainfo/metainfo.hpp"
#include "bencode/decode.hpp"

using namespace std::string_literals;

std::vector<std::string> MessageHandler::parse_server_response(std::string response)
{
    Decode decode = Decode();
    auto decoded_response = decode.decode_bencoded_value(response);
    std::string peers_str = decoded_response["peers"].get<std::string>();

    std::vector<std::string> result; // represents the the peers IP addresses

    size_t number_of_peers = peers_str.size() / 6;
    for (size_t i = 0; i < number_of_peers; i++)
    {
        std::string ip = MessageHandler::parse_ip(peers_str, i);
        std::string port = MessageHandler::parse_port(peers_str, i);

        result.push_back(ip + ":" + port);
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

std::vector<uint8_t> MessageHandler::create_handshake_message(MetaInfo metaInfo)
{
    std::string handshake_message = "\x13"s                               // length of the protocol string
                                    + "BitTorrent protocol"s              // protocol string
                                    + "\x00\x00\x00\x00\x00\x00\x00\x00"s // reserved
                                    + metaInfo.get_info_string()          // info hash
                                    + "00112233445566778899"s;            // peer id

    std::vector<uint8_t> handshake_message_bytes(handshake_message.begin(), handshake_message.end());

    return handshake_message_bytes;
}

std::vector<uint8_t> MessageHandler::create_interested_message()
{
    // const uint8_t INTERESTED_MESSAGE_ID = 2;
    std::vector<uint8_t> interested_message;
    std::vector<uint8_t> payload;
    uint32_t total_length = 4                 // message length (4 bytes)
                            + 1               // message ID (1 byte)
                            + payload.size(); // payload length (variable)

    interested_message.reserve(total_length);

    // actual message length
    uint32_t networkLength = htonl(total_length - 4);

    const uint8_t *lengthBytes = reinterpret_cast<const uint8_t *>(&networkLength);
    interested_message.insert(interested_message.end(), lengthBytes, lengthBytes + sizeof(networkLength));

    interested_message.push_back(static_cast<uint8_t>(MessageType::INTERESTED));

    interested_message.insert(interested_message.end(), payload.begin(), payload.end());

    return interested_message;
}

std::vector<uint8_t> MessageHandler::create_request_message(uint32_t index, uint32_t begin, uint32_t length)
{
    std::vector<uint8_t> request_message;
    std::vector<uint8_t> request_payload;
    // const uint8_t REQUEST_MESSAGE_ID = 6;
    uint32_t index_n = htonl(index);
    uint32_t begin_n = htonl(begin);
    uint32_t length_n = htonl(length);

    uint32_t total_length = 4                       // message length (4 bytes)
                            + 1                     // message ID (1 byte)
                            + 3 * sizeof(uint32_t); // payload length (3 * 4 bytes)
    request_message.reserve(total_length);

    // actual message length
    uint32_t networkLength = htonl(total_length - 4);

    const uint8_t *messageLengthBytes = reinterpret_cast<const uint8_t *>(&networkLength);
    request_message.insert(request_message.end(), messageLengthBytes, messageLengthBytes + sizeof(networkLength));

    request_message.push_back(static_cast<uint8_t>(MessageType::REQUEST));

    // add index_n , begin_n, length_n to the payload
    request_payload.reserve(3 * sizeof(uint32_t));

    const uint8_t *indexBytes = reinterpret_cast<const uint8_t *>(&index_n);
    const uint8_t *beginBytes = reinterpret_cast<const uint8_t *>(&begin_n);
    const uint8_t *lengthBytes = reinterpret_cast<const uint8_t *>(&length_n);

    request_payload.insert(request_payload.end(), indexBytes, indexBytes + sizeof(index_n));
    request_payload.insert(request_payload.end(), beginBytes, beginBytes + sizeof(begin_n));
    request_payload.insert(request_payload.end(), lengthBytes, lengthBytes + sizeof(length_n));

    request_message.insert(request_message.end(), request_payload.begin(), request_payload.end());

    return request_message;
}