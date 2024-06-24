#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "bencode/decode.hpp"

using json = nlohmann::json;

json Decode::decode_bencoded_value(const std::string &encoded_value)
{
    auto it = encoded_value.begin();
    return this->decode_bencoded_value(encoded_value, it);
}

json Decode::decode_bencoded_value(const std::string &encoded_value, std::string::const_iterator &it)
{
    EncodedValueType type = this->get_encoded_value_type(it);
    switch (type)
    {
    case EncodedValueType::Integer:
        return this->decode_integer(encoded_value, it);
        break;
    case EncodedValueType::String:
        return this->decode_string(encoded_value, it);
        break;
    case EncodedValueType::List:
        return this->decode_list(encoded_value, it);
        break;
    case EncodedValueType::Dict:
        return this->decode_dict(encoded_value, it);
        break;
    default:
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

EncodedValueType Decode::get_encoded_value_type(std::string::const_iterator &it)
{
    if (this->is_integer(it))
    {
        return EncodedValueType::Integer;
    }
    else if (this->is_string(it))
    {
        return EncodedValueType::String;
    }
    else if (this->is_list(it))
    {
        return EncodedValueType::List;
    }
    else if (this->is_dict(it))
    {
        return EncodedValueType::Dict;
    }
    else
    {
        return EncodedValueType::Unknown;
    }
}

bool Decode::is_integer(std::string::const_iterator &it)
{
    return *it == 'i';
}

bool Decode::is_string(std::string::const_iterator &it)
{
    return std::isdigit(*it);
}

bool Decode::is_list(std::string::const_iterator &it)
{
    return *it == 'l';
}

bool Decode::is_dict(std::string::const_iterator &it)
{
    return *it == 'd';
}

json Decode::decode_integer(const std::string &encoded_value, std::string::const_iterator &it)
{
    it++;                                                         // skip (i) char
    auto end_index = std::find(it, std::end(encoded_value), 'e'); // find index of (e) char

    if (end_index == std::end(encoded_value))
    {
        throw std::runtime_error("Integer must start with 'i' and end with 'e' -> " + encoded_value);
    }

    if (*it == '0' && std::distance(it, end_index) > 1)
    {
        throw std::runtime_error("Integer cannot have leading zeros -> " + encoded_value);
    }

    if (*it == '-' && (it + 1) != std::end(encoded_value) &&
        *(it + 1) == '0')
    {
        throw std::runtime_error("Integer cannot have negative zero -> " + encoded_value);
    }

    auto begin = (*it == '-') ? it + 1 : it;
    auto digits = std::all_of(begin, end_index, isdigit);
    if (!digits)
    {
        throw std::runtime_error("Integer must contain only digits -> " + encoded_value);
    }

    auto number_string = std::string(it, end_index);
    int64_t number = std::atoll(number_string.c_str());
    it = end_index + 1;
    return json(number);
}

json Decode::decode_string(const std::string &encoded_value, std::string::const_iterator &it)
{
    auto colon_index = std::find(it, std::end(encoded_value), ':');

    if (colon_index == std::end(encoded_value))
    {
        throw std::runtime_error("String must be in the following format (size:characters) -> " + encoded_value);
    }

    std::string number_string = std::string(it, colon_index);
    int64_t number = std::atoll(number_string.c_str());

    if (number < 0)
    {
        throw std::runtime_error("Number must be greater than or equal to 0 -> " + encoded_value);
    }

    std::string str =
        std::string(colon_index + 1, colon_index + 1 + number);

    it = colon_index + number + 1;
    return json(str);
}

json Decode::decode_list(const std::string &encoded_value, std::string::const_iterator &it)
{
    it++; // skip (l) char
    auto list = json::array();

    while (it != std::end(encoded_value) && *it != 'e')
    {
        auto val = this->decode_bencoded_value(encoded_value, it);
        list.push_back(val);
    }

    if (it == std::end(encoded_value) || *it != 'e')
    {
        throw std::runtime_error("List must start with 'l' and end with 'e' -> " + encoded_value);
    }

    it++;
    return list;
}

json Decode::decode_dict(const std::string &encoded_value, std::string::const_iterator &it)
{
    it++; // skip (d) char
    auto dict = json::object();

    while (it != std::end(encoded_value) && *it != 'e')
    {
        auto key = this->decode_bencoded_value(encoded_value, it);

        if (key.is_string() == false)
        {
            throw std::runtime_error("keys in Dict must be of type string -> " + encoded_value);
        }

        auto val = this->decode_bencoded_value(encoded_value, it);

        dict[key] = val;
    }
    if (it == std::end(encoded_value) || *it != 'e')
    {
        throw std::runtime_error("Dict must start with 'd' and end with 'e' -> " + encoded_value);
    }
    it++;
    return dict;
}