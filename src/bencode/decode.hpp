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

    bool is_dict(const std::string &encoded_value);

    std::string get_string_value(const std::string &encoded_value, size_t &index);

    std::string get_integer_value(const std::string &encoded_value, size_t &index);

    std::string get_list_or_dict_value(const std::string &encoded_value, size_t &index);

    json get_nested_element(const std::string &encoded_value, size_t &index);

    json decode_integer(const std::string &encoded_value);

    json decode_string(const std::string &encoded_value);

    json decode_list(const std::string &encoded_value);

    json decode_dict(const std::string &encoded_value);

    size_t get_char_count(const std::string &str, size_t bytes_count);

public:
    json decode_bencoded_value(const std::string &encoded_value);
};
