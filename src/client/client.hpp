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
     * @brief Construct a new Client object
     */
    Client();

    /**
     * @brief sends a get request to the tracker server to discover peers IP addresses
     *
     * @return std::string
     */
    std::string discover_peers(MetaInfo metaInfo);

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
     * @brief returns the peer id
     *
     * @return std::string
     */
    std::string get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);
};