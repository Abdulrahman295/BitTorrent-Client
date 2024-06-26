#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "metainfo/metainfo.hpp"
#include "messageHandler/message.hpp"
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
     * @return std::vector<std::string>
     */
    std::vector<std::string> discover_peers(MetaInfo metaInfo);

    /**
     * @brief creates a TCP connection with a peer
     *
     * @param metaInfo
     * @param peer_ip
     * @param peer_port
     */
    void create_connection(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

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
     * @brief receives a peer messageS over the TCP connection
     *
     * @return Message object
     */
    Message receive_peer_message();

    /**
     * @brief creates a TCP connection with a peer and returns its id
     *
     * @return std::string
     */
    std::string get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

    /**
     * @brief sends a request message and waits for a piece message for each block then returns all the blocks of a piece with the given index
     *
     * @param metaInfo
     * @return std::vector<uint8_t>
     */
    std::vector<uint8_t> fetch_piece_blocks(MetaInfo metaInfo, size_t piece_index);

    /**
     * @brief saves the data to the output file
     *
     * @param output_file
     * @param data
     */
    void save_to_file(std::string output_file, std::vector<uint8_t> data);

    /**
     * @brief downloads a piece from the peer to the output file
     *
     * @param metaInfo
     */
    void download_piece(MetaInfo metaInfo, std::string output_file, size_t piece_index);

    /**
     * @brief closes the TCP connection
     */
    void close_connection();
};