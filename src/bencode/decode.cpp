#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <stack>

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
    case EncodedValueType::List:
        return this->decode_list(encoded_value);
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
    else if (this->is_list(encoded_value))
    {
        return EncodedValueType::List;
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

bool Decode::is_list(const std::string &encoded_value)
{
    return encoded_value[0] == 'l';
}

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
        std::string value = "";
        // split the list into individual values, and decode each value
        if (encoded_value[i] == 'i') // current value is an integer
        {
            size_t end = encoded_value.find('e', i);
            if (end == encoded_value.size() - 1)
            {
                throw std::runtime_error("Integer value must end with 'e' -> " + encoded_value);
            }
            value = encoded_value.substr(i, end - i + 1);
            i = end;
        }
        else if (std::isdigit(encoded_value[i])) // current value is a string
        {
            int64_t number = std::atoll(encoded_value.substr(i, 1).c_str());
            value = encoded_value.substr(i, number + 2);
            i += number + 1;
        }
        else if (encoded_value[i] == 'l') // current value is a list
        {
            // find the end of the list and decode the list.
            // use stack to find the end of the list.
            std::stack<char> stack;
            stack.push(encoded_value[i]);
            for (size_t j = i + 1; j < encoded_value.size() - 1; j++)
            {
                if (std::isdigit(encoded_value[j]))
                {
                    int64_t number = std::atoll(encoded_value.substr(j, 1).c_str());
                    j += number + 1;
                }
                else if (encoded_value[j] == 'i')
                {
                    size_t end = encoded_value.find('e', j);
                    j = end;
                }
                else if (encoded_value[j] == 'l')
                {
                    stack.push(encoded_value[j]);
                }
                else if (encoded_value[j] == 'e')
                {
                    stack.pop();
                    if (stack.empty())
                    {
                        value = encoded_value.substr(i, j - i + 1);
                        i = j;
                        break;
                    }
                }
            }

            if (!stack.empty())
            {
                throw std::runtime_error("List must end with 'e' -> " + encoded_value);
            }
        }
        else
        {
            throw std::runtime_error("Invalid value in list -> " + encoded_value);
        }

        list.push_back(this->decode_bencoded_value(value));
    }

    return json(list);
}
