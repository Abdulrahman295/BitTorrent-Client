#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"

#include "bencode/decode.hpp"
#include "metainfo/metainfo.hpp"

using json = nlohmann::json;

/**
 * @brief handles the decode command
 *
 * @param argc
 * @param argv
 * @return int
 */
int decode_command(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
        return 1;
    }

    std::string encoded_value = argv[2];

    try
    {
        Decode decode = Decode();
        std::cout << decode.decode_bencoded_value(encoded_value).dump() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

/**
 * @brief handles the info command
 *
 * @param argc
 * @param argv
 * @return int
 */
int info_command(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " info <torrent_file>" << std::endl;
        return 1;
    }

    std::string torrent_file = argv[2];

    try
    {
        MetaInfo meta_info = MetaInfo(torrent_file);
        std::cout << "Tracker URL: " << meta_info.get_announceURL() << std::endl;
        std::cout << "Length: " << meta_info.get_file_size() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 2)
    {
        std::cerr << "Usage: \t " << argv[0] << " decode <encoded_value>" << std::endl;
        std::cerr << "\t " << argv[0] << " info <torrent file>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "decode")
    {
        return decode_command(argc, argv);
    }
    else if (command == "info")
    {
        return info_command(argc, argv);
    }
    else
    {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }
}