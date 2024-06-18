#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "decode.hpp"


using json = nlohmann::json;

json Decode::decode_bencoded_value(const std::string &encoded_value)
{
    EncodedValueType type = this->get_encoded_value_type(encoded_value);

    switch (type)
    {
    case EncodedValueType::Integer:
        return this->decode_integer(encoded_value);
        break;
    case EncodedValueType::String:
        return this->decode_string(encoded_value);
        break;
    default:
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

EncodedValueType Decode::get_encoded_value_type(const std::string &encoded_value)
{
    if (this->is_integer(encoded_value))
    {
        return EncodedValueType::Integer;
    }
    else if (this->is_string(encoded_value))
    {
        return EncodedValueType::String;
    }
    else
    {
        return EncodedValueType::Unknown;
    }
}

bool Decode::is_integer(const std::string &encoded_value)
{
    return encoded_value[0] == 'i';
}

bool Decode::is_string(const std::string &encoded_value)
{
    return std::isdigit(encoded_value[0]);
}

json Decode::decode_integer(const std::string &encoded_value)
{
    if (encoded_value[0] != 'i' || encoded_value[encoded_value.size() - 1] != 'e')
    {
        throw std::runtime_error("Integer must start with 'i' and end with 'e' -> " + encoded_value);
    }

    // if there is no number between 'i' and 'e' then it is invalid
    if (encoded_value.size() == 2)
    {
        throw std::runtime_error("Integer must have a number between 'i' and 'e' -> " + encoded_value);
    }

    // if there is a leading zero then it is invalid
    if (encoded_value[1] == '0' && encoded_value[2] != 'e')
    {
        throw std::runtime_error("Integer cannot have leading zeros -> " + encoded_value);
    }

    // if there is a negative zero then it is invalid
    if (encoded_value[1] == '-' && encoded_value[2] == '0')
    {
        throw std::runtime_error("Integer cannot have negative zero -> " + encoded_value);
    }

    std::string number_string = encoded_value.substr(1, encoded_value.size() - 2);
    int64_t number = std::atoll(number_string.c_str());
    return json(number);
}

json Decode::decode_string(const std::string &encoded_value)
{
    size_t colon_index = encoded_value.find(':');

    if (colon_index == std::string::npos)
    {
        throw std::runtime_error("String must be in the following format (size:characters) -> " + encoded_value);
    }

    std::string number_string = encoded_value.substr(0, colon_index);
    int64_t number = std::atoll(number_string.c_str());

    if (number < 0)
    {
        throw std::runtime_error("Number must be greater than or equal to 0 -> " + encoded_value);
    }

    if (encoded_value.size() - colon_index - 1 != number)
    {
        throw std::runtime_error("Number of characters does not match the size -> " + encoded_value);
    }

    std::string str = encoded_value.substr(colon_index + 1);
    return json(str);
}
