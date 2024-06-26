#pragma once

#include <string>
#include <vector>

#include "metainfo/metainfo.hpp"
#include "messageHandler/message.hpp"

class MessageHandler
{
public:
    /**
     * @brief parse the tracker response and return the peers IP addresses
     *
     * @param response
     * @return std::vector<std::string>
     */
    static std::vector<std::string> parse_server_response(std::string response);

    /**
     * @brief parse the IP address from the peers string
     *
     * @param peers_str
     * @param index
     * @return std::string
     */
    static std::string parse_ip(std::string peers_str, size_t index);

    /**
     * @brief parse the port number from the peers string
     *
     * @param peers_str
     * @param index
     * @return std::string
     */
    static std::string parse_port(std::string peers_str, size_t index);

    /**
     * @brief parse the peer response and return the peer id
     *
     * @param response
     * @return std::string
     */
    static std::string parse_handshake_response(std::string response);

    /**
     * @brief creates a handshake message to send to the peer
     *
     * @param metaInfo
     * @return std::string
     */
    static std::vector<uint8_t> create_handshake_message(MetaInfo metaInfo);

    /**
     * @brief creates an interested message to send to the peer, where message id is 2 and payload is empty
     *
     * @return std::vector<uint8_t>
     */
    static std::vector<uint8_t> create_interested_message();

    /**
     * @brief creates a request message to send to the peer, where message id is 3 and payload contains the piece index, begin offset and length
     *
     * @return std::vector<uint8_t>
     */
    static std::vector<uint8_t> create_request_message(uint32_t index, uint32_t begin, uint32_t length);
};
