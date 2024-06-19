#include <string>

#include "../lib/nlohmann/json.hpp"
#include "type.hpp"

using json = nlohmann::json;

class Decode
{
private:
    EncodedValueType get_encoded_value_type(const std::string &encoded_value);

    bool is_integer(const std::string &encoded_value);

    bool is_string(const std::string &encoded_value);

    bool is_list(const std::string &encoded_value);

    json decode_integer(const std::string &encoded_value);

    json decode_string(const std::string &encoded_value);

    json decode_list(const std::string &encoded_value);

public:
    json decode_bencoded_value(const std::string &encoded_value);
};
