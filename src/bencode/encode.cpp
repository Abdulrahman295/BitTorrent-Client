#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "encode.hpp"
#include "../lib/nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @brief Encodes the json value based on the type of the value
 *
 * @param value
 * @return std::string
 */
std::string Encode::encode_bencoded_value(const json &value)
{
    if (value.is_number_integer())
    {
        return this->encode_integer(value);
    }
    else if (value.is_string())
    {
        return this->encode_string(value);
    }
    else if (value.is_array())
    {
        return this->encode_list(value);
    }
    else if (value.is_object())
    {
        return this->encode_dict(value);
    }
    else
    {
        throw std::runtime_error("Unhandled json value: " + value.dump());
    }
}

/**
 * @brief Encodes the integer value
 *
 * @param value
 * @return std::string
 */
std::string Encode::encode_integer(const json &value)
{
    return "i" + value.dump() + "e";
}

/**
 * @brief Encodes the string value
 *
 * @param value
 * @return std::string
 */
std::string Encode::encode_string(const json &value)
{

    return std::to_string(value.get<std::string>().size()) + ":" + value.get<std::string>();
}

/**
 * @brief Encodes the list value
 *
 * @param value
 * @return std::string
 */
std::string Encode::encode_list(const json &value)
{
    std::string encoded_value = "l";

    for (auto &element : value)
    {
        encoded_value += this->encode_bencoded_value(element);
    }

    encoded_value += "e";
    return encoded_value;
}

/**
 * @brief Encodes the dict value
 *
 * @param value
 * @return std::string
 */
std::string Encode::encode_dict(const json &value)
{
    std::string encoded_value = "d";

    for (auto it = value.begin(); it != value.end(); ++it)
    {
        encoded_value += this->encode_string(it.key());
        encoded_value += this->encode_bencoded_value(it.value());
    }

    encoded_value += "e";
    return encoded_value;
}