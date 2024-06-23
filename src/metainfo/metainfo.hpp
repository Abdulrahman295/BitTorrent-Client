#pragma once

#include <string>
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

    std::string read_file(std::filesystem::path torrent_file);

    std::string get_announceURL();

    size_t get_file_size();

    std::string get_name();

    size_t get_piece_length();

    std::string get_pieces_hash();

    std::string to_string();

    json to_json();

    std::string get_info_hash();

    std::string get_info_string();

    std::string stringToHex(const std::string &input);
};