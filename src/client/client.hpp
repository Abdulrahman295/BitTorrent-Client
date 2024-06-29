#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <queue>
#include <mutex>

#include "metainfo/metainfo.hpp"
#include "messageHandler/message.hpp"
#include "client/connection.hpp"
class Client
{
private:
    std::queue<size_t> work_queue;
    std::mutex work_queue_mutex;
    std::vector<std::vector<uint8_t>> downloaded_pieces;
    std::mutex downloaded_pieces_mutex;

public:
    /**
     * @brief sends a get request to the tracker server to discover peers IP addresses
     *
     * @return std::vector<std::string>
     */
    std::vector<std::string> discover_peers(MetaInfo metaInfo);

    /**
     * @brief exchanges hanshake message with a peer and returns its id
     *
     * @return std::string
     */
    std::string get_peer_id(MetaInfo metaInfo, Connection &peerConnection);

    /**
     * @brief initiates a connection with a peer to be ready for requesting pieces
     *
     * @param metaInfo
     * @param peer_ip
     * @param peer_port
     * @return Connection object
     */
    Connection connect_to_peer(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

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
     * @brief picks a piece from the work queue and downloads it from the peer
     * @param metaInfo
     * @param peer_ip
     * @param peer_port
     */
    void worker(MetaInfo metaInfo, const std::string peer_ip, const std::string peer_port);

    /**
     * @brief downloads the entire file from available peers
     *
     * @param metaInfo
     * @param output_file
     */
    void download_file(MetaInfo metaInfo, std::string output_file);
};