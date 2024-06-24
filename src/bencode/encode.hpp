#pragma once

#include <string>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

class Encode
{
private:
    /**
     * @brief Encodes the integer value, example: 123 -> i123e
     *
     * @param value
     * @return std::string
     */
    std::string encode_integer(const json &value);

    /**
     * @brief Encodes the string value, example: "hello" -> 5:hello
     *
     * @param value
     * @return std::string
     */
    std::string encode_string(const json &value);

    /**
     * @brief Encodes the list value, example: [1, 2, 3] -> li1ei2ei3ee
     *
     * @param value
     * @return std::string
     */
    std::string encode_list(const json &value);

    /**
     * @brief Encodes the dict value, example: {"key": "value"} -> d3:key5:valuee
     *
     * @param value
     * @return std::string
     */
    std::string encode_dict(const json &value);

public:
    /**
     * @brief Encodes the json value based on the type of the value
     *
     * @param value
     * @return std::string
     */
    std::string encode_bencoded_value(const json &value);
};