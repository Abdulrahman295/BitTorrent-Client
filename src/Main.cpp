#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"
#include <cpr/cpr.h>

#include "bencode/decode.hpp"
#include "bencode/encode.hpp"
#include "metainfo/metainfo.hpp"
#include "client/client.hpp"

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
        std::cout << "Info Hash: " << meta_info.get_info_hash() << std::endl;
        std::cout << "Piece Length: " << meta_info.get_piece_length() << std::endl;
        std::cout << "Piece Hashes: \n"
                  << meta_info.get_pieces_hash() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

/**
 * @brief handles the peers command
 *
 * @param argc
 * @param argv
 * @return int
 */
int peers_command(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " peers <torrent_file>" << std::endl;
        return 1;
    }

    std::string torrent_file = argv[2];

    try
    {
        MetaInfo metaInfo = MetaInfo(torrent_file);
        Client cli = Client();
        std::string peers = cli.discover_peers(metaInfo);
        std::cout << peers << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}

/**
 * @brief handles the handshake command
 *
 * @param argc
 * @param argv
 * @return int
 */
int handshake_command(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " handshake <torrent_file> <peer_ip>:<peer_port>" << std::endl;
        return 1;
    }

    std::string torrent_file = argv[2];

    try
    {
        std::string address = argv[3];
        std::string peer_ip = address.substr(0, address.find(":"));
        std::string peer_port = address.substr(address.find(":") + 1);

        MetaInfo metaInfo = MetaInfo(torrent_file);
        Client cli = Client();
        std::string peerID = cli.get_peer_id(metaInfo, peer_ip, peer_port);
        std::cout << "Peer ID: " << peerID << std::endl;
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
    else if (command == "peers")
    {
        return peers_command(argc, argv);
    }
    else if (command == "handshake")
    {
        return handshake_command(argc, argv);
    }
    else
    {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }
}