#pragma once

#include <string>

#include "metainfo/metainfo.hpp"

class Client
{
private:
    SOCKET sock;

public:
    std::string discover_peers(MetaInfo metaInfo);

    std::string parse_server_response(std::string response);

    std::string parse_ip(std::string peers_str, size_t index);

    std::string parse_port(std::string peers_str, size_t index);

    void create_connection(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);

    std::string create_handshake_message(MetaInfo metaInfo);

    void send_message(std::string message);

    std::string receive_message();

    std::string parse_peer_response(std::string response);

    std::string get_peer_id(MetaInfo metaInfo, std::string peer_ip, std::string peer_port);
};