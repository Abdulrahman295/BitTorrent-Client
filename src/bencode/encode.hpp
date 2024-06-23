#pragma once

#include <string>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

class Encode
{
private:
    std::string encode_integer(const json &value);

    std::string encode_string(const json &value);

    std::string encode_list(const json &value);

    std::string encode_dict(const json &value);

public:
    std::string encode_bencoded_value(const json &value);
};