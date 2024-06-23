#pragma once

#include <string>

#include "metainfo/metainfo.hpp"

class Client
{

public:
    std::string discover_peers(MetaInfo metaInfo);

    std::string parse_tracker_response(std::string response);

    std::string parse_ip(std::string peers_str, size_t index);

    std::string parse_port(std::string peers_str, size_t index);
};