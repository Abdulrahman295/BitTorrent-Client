#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "metainfo.hpp"
#include "../bencode/decode.hpp"

MetaInfo::MetaInfo(std::filesystem::path torrent_file)
{
    std::string encoded_value = this->read_file(torrent_file);
    Decode decode = Decode();
    json info = decode.decode_bencoded_value(encoded_value);
    this->announceURL = info["announce"];
    this->file_size = info["info"]["length"];
    this->name = info["info"]["name"];
    this->piece_length = info["info"]["piece length"];
    this->pieces_hash = info["info"]["pieces"];
}

/**
 * @brief read_file reads the content of the torrent file
 *
 * @param torrent_file
 * @return std::string
 */
std::string MetaInfo::read_file(std::filesystem::path torrent_file)
{
    std::ifstream file(torrent_file, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + torrent_file.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string encoded_value = buffer.str();
    file.close();

    // normalize the string characters
    size_t pos = 0;
    while ((pos = encoded_value.find("\r\n", pos)) != std::string::npos)
    {
        encoded_value.replace(pos, 2, "\n");
        pos += 1; // Move past the newly inserted character
    }

    return encoded_value;
}

/**
 * @brief returns the announce URL
 *
 * @return std::string
 */
std::string MetaInfo::get_announceURL()
{
    return this->announceURL;
}

/**
 * @brief returns the file size
 *
 * @return size_t
 */
size_t MetaInfo::get_file_size()
{
    return this->file_size;
}

/**
 * @brief returns the name of the file
 *
 * @return std::string
 */
std::string MetaInfo::get_name()
{
    return this->name;
}

/**
 * @brief returns the piece length
 *
 * @return size_t
 */
size_t MetaInfo::get_piece_length()
{
    return this->piece_length;
}

/**
 * @brief returns the pieces hash string
 *
 * @return std::string
 */
std::string MetaInfo::get_pieces_hash()
{
    return this->pieces_hash;
}

/**
 * @brief returns the string representation of the MetaInfo object
 *
 * @return std::string
 */
std::string MetaInfo::to_string()
{
    std::string str = "Announce URL: " + this->announceURL + "\n";
    str += "File size: " + std::to_string(this->file_size) + "\n";
    str += "Name: " + this->name + "\n";
    str += "Piece length: " + std::to_string(this->piece_length) + "\n";
    str += "Pieces hash: " + this->pieces_hash + "\n";
    return str;
}

/**
 * @brief returns the json representation of the MetaInfo object
 *
 * @return json
 */
json MetaInfo::to_json()
{
    json j;
    j["announceURL"] = this->announceURL;
    j["file_size"] = this->file_size;
    j["name"] = this->name;
    j["piece_length"] = this->piece_length;
    j["pieces_hash"] = this->pieces_hash;
    return j;
}