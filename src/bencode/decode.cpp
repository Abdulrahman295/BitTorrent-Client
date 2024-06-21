#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <stack>

#include "decode.hpp"

using json = nlohmann::json;

/**
 * @brief  Decodes the bencoded value based on the type of the value
 *
 * @param encoded_value
 * @return json
 */
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
    case EncodedValueType::List:
        return this->decode_list(encoded_value);
        break;
    case EncodedValueType::Dict:
        return this->decode_dict(encoded_value);
        break;
    default:
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

/**
 * @brief Returns the type of the encoded value
 *
 * @param encoded_value
 * @return EncodedValueType
 */
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
    else if (this->is_list(encoded_value))
    {
        return EncodedValueType::List;
    }
    else if (this->is_dict(encoded_value))
    {
        return EncodedValueType::Dict;
    }
    else
    {
        return EncodedValueType::Unknown;
    }
}

/**
 * @brief returns true if the encoded value is an integer
 *
 * @param encoded_value
 * @return true
 * @return false
 */
bool Decode::is_integer(const std::string &encoded_value)
{
    return encoded_value[0] == 'i';
}

/**
 * @brief returns true if the encoded value is a string
 *
 * @param encoded_value
 * @return true
 * @return false
 */
bool Decode::is_string(const std::string &encoded_value)
{
    return std::isdigit(encoded_value[0]);
}

/**
 * @brief returns true if the encoded value is a list
 *
 * @param encoded_value
 * @return true
 * @return false
 */
bool Decode::is_list(const std::string &encoded_value)
{
    return encoded_value[0] == 'l';
}

/**
 * @brief returns true if the encoded value is a dict
 *
 * @param encoded_value
 * @return true
 * @return false
 */
bool Decode::is_dict(const std::string &encoded_value)
{
    return encoded_value[0] == 'd';
}

/**
 * @brief return the bencoded string value in the format (size:characters) from a sequence of characters
 *
 * @param encoded_value
 * @param index
 * @return std::string
 */
std::string Decode::get_string_value(const std::string &encoded_value, size_t &index)
{
    size_t colon_index = encoded_value.find(':', index);

    if (colon_index == std::string::npos)
    {
        throw std::runtime_error("String must be in the following format (size:characters) -> " + encoded_value);
    }

    std::string number_string = encoded_value.substr(index, colon_index - index);
    int64_t number = std::atoll(number_string.c_str());
    size_t char_count = this->get_char_count(encoded_value.substr(colon_index + 1), number);
    std::string value = number_string + encoded_value.substr(colon_index, char_count + 1);
    index = colon_index + char_count;
    return value;
}

/**
 * @brief return the bencoded integer value in the format (i<number>e) from a sequence of characters
 *
 * @param encoded_value
 * @param index
 * @return std::string
 */
std::string Decode::get_integer_value(const std::string &encoded_value, size_t &index)
{
    size_t end = encoded_value.find('e', index);
    if (end == encoded_value.size() - 1)
    {
        throw std::runtime_error("Integer value must end with 'e' -> " + encoded_value);
    }
    std::string value = encoded_value.substr(index, end - index + 1);
    index = end;
    return value;
}

/**
 * @brief return the bencoded list or dict value in the format (l<value>e or d<value>e) from a sequence of characters
 *
 * @param encoded_value
 * @param index
 * @return std::string
 */
std::string Decode::get_list_or_dict_value(const std::string &encoded_value, size_t &index)
{
    std::stack<char> stack;
    std::string value = "";
    stack.push(encoded_value[index]);

    for (size_t j = index + 1; j < encoded_value.size() - 1; j++)
    {
        if (std::isdigit(encoded_value[j]))
        {
            size_t colon_index = encoded_value.find(':', j);

            if (colon_index == std::string::npos)
            {
                throw std::runtime_error("String must be in the following format (size:characters) -> " + encoded_value);
            }

            std::string number_string = encoded_value.substr(j, colon_index - j);
            int64_t number = std::atoll(number_string.c_str());
            size_t char_count = this->get_char_count(encoded_value.substr(colon_index + 1), number);
            j = colon_index + char_count;
        }
        else if (encoded_value[j] == 'i')
        {
            size_t end = encoded_value.find('e', j);
            if (end == encoded_value.size() - 1)
            {
                throw std::runtime_error("Integer value must end with 'e' -> " + encoded_value);
            }
            j = end;
        }
        else if (encoded_value[j] == 'l' || encoded_value[j] == 'd')
        {
            stack.push(encoded_value[j]);
        }
        else if (encoded_value[j] == 'e')
        {
            stack.pop();
            if (stack.empty())
            {
                value = encoded_value.substr(index, j - index + 1);
                index = j;
                break;
            }
        }
    }

    if (!stack.empty())
    {
        throw std::runtime_error("List/Dict must end with 'e' -> " + encoded_value);
    }

    return value;
}
/**
 * @brief returns the json object of individual nested elements inside a list or dict
 *
 * @param encoded_value
 * @param index
 * @return json
 */
json Decode::get_nested_element(const std::string &encoded_value, size_t &index)
{
    std::string value = "";
    if (encoded_value[index] == 'i') // current value is an integer
    {
        value = this->get_integer_value(encoded_value, index);
    }
    else if (std::isdigit(encoded_value[index])) // current value is a string
    {
        value = this->get_string_value(encoded_value, index);
    }
    else if (encoded_value[index] == 'l' || encoded_value[index] == 'd') // current value is a list or dict
    {
        value = this->get_list_or_dict_value(encoded_value, index);
    }
    else
    {
        throw std::runtime_error("Invalid encoded value -> " + encoded_value);
    }

    return this->decode_bencoded_value(value);
}

/**
 * @brief returns the json object of an integer value
 *
 * @param encoded_value
 * @return json
 */
json Decode::decode_integer(const std::string &encoded_value)
{
    if (encoded_value[0] != 'i' || encoded_value[encoded_value.size() - 1] != 'e')
    {
        throw std::runtime_error("Integer must start with 'i' and end with 'e' -> " + encoded_value);
    }

    if (encoded_value.size() == 2)
    {
        throw std::runtime_error("Integer must have a number between 'i' and 'e' -> " + encoded_value);
    }

    if (encoded_value[1] == '0' && encoded_value[2] != 'e')
    {
        throw std::runtime_error("Integer cannot have leading zeros -> " + encoded_value);
    }

    if (encoded_value[1] == '-' && encoded_value[2] == '0')
    {
        throw std::runtime_error("Integer cannot have negative zero -> " + encoded_value);
    }

    for (size_t i = 1; i < encoded_value.size() - 1; i++)
    {
        if (i == 1 && encoded_value[i] == '-')
        {
            continue;
        }
        if (!std::isdigit(encoded_value[i]))
        {
            throw std::runtime_error("Integer must contain only digits -> " + encoded_value);
        }
    }

    std::string number_string = encoded_value.substr(1, encoded_value.size() - 2);
    int64_t number = std::atoll(number_string.c_str());
    return json(number);
}

/**
 * @brief returns the json object of a string value
 *
 * @param encoded_value
 * @return json
 */
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

    size_t char_count = this->get_char_count(encoded_value.substr(colon_index + 1), number);

    if (encoded_value.size() - colon_index - 1 != char_count)
    {
        throw std::runtime_error("Number of characters does not match the size -> " + encoded_value);
    }

    std::string str = encoded_value.substr(colon_index + 1);
    return json(str);
}

/**
 * @brief returns the json object of a list value
 *
 * @param encoded_value
 * @return json
 */
json Decode::decode_list(const std::string &encoded_value)
{
    if (encoded_value[0] != 'l' || encoded_value[encoded_value.size() - 1] != 'e')
    {
        throw std::runtime_error("List must start with 'l' and end with 'e' -> " + encoded_value);
    }
    // if there is no value between 'l' and 'e', return an empty list
    if (encoded_value.size() == 2)
    {
        return json(std::vector<json>());
    }

    std::vector<json> list;

    for (size_t i = 1; i < encoded_value.size() - 1; i++)
    {
        list.push_back(this->get_nested_element(encoded_value, i));
    }

    return json(list);
}

/**
 * @brief returns the json object of a dict value
 *
 * @param encoded_value
 * @return json
 */
json Decode::decode_dict(const std::string &encoded_value)
{
    if (encoded_value[0] != 'd' || encoded_value[encoded_value.size() - 1] != 'e')
    {
        throw std::runtime_error("Dict must start with 'd' and end with 'e' -> " + encoded_value);
    }

    // if there is no value between 'd' and 'e', return an empty dict
    if (encoded_value.size() == 2)
    {
        return json(std::map<std::string, json>());
    }

    std::map<std::string, json> dict;

    for (size_t i = 1; i < encoded_value.size() - 1; i++)
    {
        std::string key = "";
        std::string value = "";
        // split the dict into individual key-value pairs, and decode each key-value pair
        if (std::isdigit(encoded_value[i]))
        {
            // get the key and decode the value
            key = this->get_string_value(encoded_value, i);
            size_t colon_index = key.find(':');
            key = key.substr(colon_index + 1);
            i++;
            dict[key] = this->get_nested_element(encoded_value, i);
        }
        else
        {
            throw std::runtime_error("Invalid key in dict -> " + encoded_value);
        }
    }

    return json(dict);
}

/**
 * @brief returns the number of characters in a string value based on the number of bytes handles non Ascii characters
 *
 * @param str
 * @param bytes_count
 * @return size_t
 */
size_t Decode::get_char_count(const std::string &str, size_t bytes_count)
{
    size_t current_bytes_count = 0;
    size_t char_count = 0;

    for (size_t i = 0; i < str.size() && current_bytes_count < bytes_count; ++i)
    {

        // if it marks the start of a byte sequence in UTF-8, increment the current_bytes_count
        if ((str[i] & 0xC0) != 0x80)
        {
            ++current_bytes_count;
        }

        ++char_count;
    }

    return char_count + bytes_count - current_bytes_count;
}