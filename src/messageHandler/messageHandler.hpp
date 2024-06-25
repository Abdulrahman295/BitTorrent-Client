#pragma once

#include <string>

#include "metainfo/metainfo.hpp"

class MessageHandler
{
public:
    /**
     * @brief parse the tracker response and return the peers IP addresses
     *
     * @param response
     * @return std::string
     */
    static std::string parse_server_response(std::string response);

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
    static std::string create_handshake_message(MetaInfo metaInfo);
};
