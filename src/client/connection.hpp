#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <vector>

#include "metainfo/metainfo.hpp"
#include "messageHandler/message.hpp"

class Connection
{
private:
    int sock;

public:
    /**
     * @brief creates a TCP connection with the peer
     *
     * @param peer_ip
     * @param peer_port
     */
    Connection(const std::string &peer_ip, const std::string &peer_port);

    /**
     * @brief destroys the TCP connection
     *
     */
    ~Connection();

    /**
     * @brief sends a message to the peer over the TCP connection
     *
     * @param message
     */
    void send_message(std::vector<uint8_t> message);

    /**
     * @brief receives handshake message from the peer over the TCP connection
     *
     * @return std::string
     */
    std::string receive_handshake_message();

    /**
     * @brief receives a peer message over the TCP connection
     *
     * @return Message object
     */
    Message receive_peer_message();

    /**
     * @brief sends a request message and waits for a piece message for each block then returns all the blocks of a piece with the given index
     *
     * @param metaInfo
     * @return std::vector<uint8_t>
     */
    std::vector<uint8_t> fetch_piece_blocks(MetaInfo metaInfo, size_t piece_index);
};
