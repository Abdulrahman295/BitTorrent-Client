#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "metainfo/metainfo.hpp"

class Client
{
private:
    int sock;

public:
    /**
     * @brief sends a get request to the tracker server to discover peers IP addresses
     *
     * @return std::string
     */
    std::string discover_peers(MetaInfo metaInfo);

    /**
     * @brief parse the tracker response and return the peers IP addresses
     *
     * @param response
     * @return std::string
     */
    std::string parse_server_response(std::string response);

    /**
     * @brief parse the IP address from the peers string
     *
     * @param peers_str
     * @param index
     * @return std::string
     */
    std::string parse_ip(std::string peers_str, size_t index);

    /**
     * @brief parse the port number from the peers string
     *
     * @param peers_str
     * @param index
     * @return std::string
     */
    std::string parse_port(std::string peers_str, size_t index);

    /**
     * @brief creates a TCP connection with a peer
     *
     * @param metaInfo
     * @param peer_ip
     * @param peer_port
     * @return std::string
     */
    void create_connection(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

    /**
     * @brief creates a handshake message to send to the peer
     *
     * @param metaInfo
     * @return std::string
     */
    std::string create_handshake_message(MetaInfo metaInfo);

    /**
     * @brief sends a message to the peer over the TCP connection
     *
     * @param message
     */
    void send_message(std::string message);

    /**
     * @brief receives a message from the peer over the TCP connection
     *
     * @return std::string
     */
    std::string receive_message();

    /**
     * @brief parse the peer response and return the peer id
     *
     * @param response
     * @return std::string
     */
    std::string parse_peer_response(std::string response);

    /**
     * @brief returns the peer id
     *
     * @return std::string
     */
    std::string get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);
};