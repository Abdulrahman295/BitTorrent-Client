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

    // /**
    //  * @brief creates a TCP connection with a peer and sends a handshake message.
    //  *
    //  * @param metaInfo
    //  * @param peer_ip
    //  * @param peer_port
    //  */
    // void do_handshake(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

    /**
     * @brief returns peer id
     *
     * @return std::string
     */
    std::string get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

    /**
     * @brief initiates a connection with a peer to be ready for requesting pieces
     *
     * @param metaInfo
     * @param peer_ip
     * @param peer_port
     */
    void connect_to_peer(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

    /**
     * @brief sends a request message and waits for a piece message for each block then returns all the blocks of a piece with the given index
     *
     * @param metaInfo
     * @return std::vector<uint8_t>
     */
    std::vector<uint8_t> fetch_piece_blocks(MetaInfo metaInfo, size_t piece_index);

    /**
     * @brief verifies the piece by comparing its hash with the expected hash
     *
     * @param metaInfo
     * @param piece_data
     * @param piece_index
     */
    void verify_piece(MetaInfo metaInfo, std::vector<uint8_t> piece_data, size_t piece_index);

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
     * @param output_file
     * @param piece_index
     */
    void download_piece(MetaInfo metaInfo, std::string output_file, size_t piece_index);

    /**
     * @brief downloads the file from the peers
     *
     * @param metaInfo
     * @param output_file
     */
    void download_file(MetaInfo metaInfo, std::string output_file);

    /**
     * @brief closes the TCP connection
     */
    void close_connection();
};