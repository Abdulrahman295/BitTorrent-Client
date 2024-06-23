#pragma once

#include <string>

#include "lib/nlohmann/json.hpp"
#include "bencode/type.hpp"

using json = nlohmann::json;

class Decode
{

private:
    EncodedValueType get_encoded_value_type(std::string::const_iterator &it);

    bool is_integer(std::string::const_iterator &it);

    bool is_string(std::string::const_iterator &it);

    bool is_list(std::string::const_iterator &it);

    bool is_dict(std::string::const_iterator &it);

    json decode_integer(const std::string &encoded_value, std::string::const_iterator &it);

    json decode_string(const std::string &encoded_value, std::string::const_iterator &it);

    json decode_list(const std::string &encoded_value, std::string::const_iterator &it);

    json decode_dict(const std::string &encoded_value, std::string::const_iterator &it);

    json decode_bencoded_value(const std::string &encoded_value, std::string::const_iterator &it);

public:
    json decode_bencoded_value(const std::string &encoded_value);
};
