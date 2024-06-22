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
#include "../bencode/encode.hpp"
#include "sha1.hpp"

MetaInfo::MetaInfo(std::filesystem::path torrent_file)
{
    std::string encoded_value = this->read_file(torrent_file);
    Decode decode = Decode();
    json metaInfo = decode.decode_bencoded_value(encoded_value);
    this->announceURL = metaInfo["announce"];
    this->file_size = metaInfo["info"]["length"];
    this->name = metaInfo["info"]["name"];
    this->piece_length = metaInfo["info"]["piece length"];
    this->pieces_hash = metaInfo["info"]["pieces"];
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
 * @brief converts a string to its hexadecimal representation
 *
 * @param input
 * @return std::string
 */
std::string MetaInfo::stringToHex(const std::string &input)
{
    std::stringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');

    for (size_t i = 0; i < input.length(); ++i)
    {
        hex_stream << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(input[i]));
    }

    return hex_stream.str();
}

/**
 * @brief returns the pieces hash string in the hexademical format
 *
 * @return std::string
 */
std::string MetaInfo::get_pieces_hash()
{
    std::string result = "";
    auto it = this->pieces_hash.begin();

    while (it != this->pieces_hash.end())
    {
        result += this->stringToHex(std::string(it, it + 20)) + "\n";
        it += 20;
    }

    return result;
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
    json j = {
        {"length", this->file_size},
        {"name", this->name},
        {"piece length", this->piece_length},
        {"pieces", this->pieces_hash}};

    return j;
}

/**
 * @brief returns the info hash of the torrent file
 *
 * @return std::string
 */
std::string MetaInfo::get_info_hash()
{
    json info = this->to_json();
    Encode encode = Encode();
    std::string encoded_info = encode.encode_bencoded_value(info);
    SHA1 checksum;
    checksum.update(encoded_info);

    return checksum.final();
    // return encoded_info;
}