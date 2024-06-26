#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

class MetaInfo
{
private:
    std::string announceURL;
    size_t file_size;
    std::string name;
    size_t piece_length;
    std::string pieces_hash;

public:
    MetaInfo(std::filesystem::path torrent_file);

    /**
     * @brief read_file reads the content of the torrent file
     *
     * @param torrent_file
     * @return std::string
     */
    std::string read_file(std::filesystem::path torrent_file);

    /**
     * @brief returns the announce URL
     *
     * @return std::string
     */
    std::string get_announceURL();

    /**
     * @brief returns the file size
     *
     * @return size_t
     */
    size_t get_file_size();

    /**
     * @brief returns the suggested name for the file
     *
     * @return std::string
     */
    std::string get_name();

    /**
     * @brief returns the piece length
     *
     * @return size_t
     */
    size_t get_piece_length();

    /**
     * @brief converts a string to its hexadecimal representation
     *
     * @param input
     * @return std::string
     */
    std::string stringToHex(const std::string &input);

    /**
     * @brief returns the pieces hash string in the hexadecimal format
     *
     * @return std::vector<std::string>
     */
    std::vector<std::string> get_pieces_hash();

    /**
     * @brief returns the string representation of the MetaInfo object
     *
     * @return std::string
     */
    std::string to_string();

    /**
     * @brief returns the json representation of the MetaInfo object
     *
     * @return json
     */
    json to_json();

    /**
     * @brief returns the info hash of the torrent file
     *
     * @return std::string
     */
    std::string get_info_hash();

    /**
     * @brief returns the info string of the torrent file in the hexadecimal format
     *
     * @return std::string
     */
    std::string get_info_string();
};