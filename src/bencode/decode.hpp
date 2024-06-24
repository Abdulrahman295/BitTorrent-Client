#pragma once

#include <string>

#include "lib/nlohmann/json.hpp"
#include "bencode/type.hpp"

using json = nlohmann::json;

class Decode
{

private:
    /**
     * @brief Returns the type of the encoded value
     *
     * @param encoded_value
     * @return EncodedValueType
     */
    EncodedValueType get_encoded_value_type(std::string::const_iterator &it);

    /**
     * @brief returns true if the encoded value is an integer
     *
     * @param encoded_value
     * @return true
     * @return false
     */
    bool is_integer(std::string::const_iterator &it);

    /**
     * @brief returns true if the encoded value is a string
     *
     * @param encoded_value
     * @return true
     * @return false
     */
    bool is_string(std::string::const_iterator &it);

    /**
     * @brief returns true if the encoded value is a list
     *
     * @param encoded_value
     * @return true
     * @return false
     */
    bool is_list(std::string::const_iterator &it);

    /**
     * @brief returns true if the encoded value is a dict
     *
     * @param encoded_value
     * @return true
     * @return false
     */
    bool is_dict(std::string::const_iterator &it);

    /**
     * @brief returns the json object of an integer value
     *
     * @param encoded_value
     * @return json
     */
    json decode_integer(const std::string &encoded_value, std::string::const_iterator &it);

    /**
     * @brief returns the json object of a string value
     *
     * @param encoded_value
     * @return json
     */
    json decode_string(const std::string &encoded_value, std::string::const_iterator &it);

    /**
     * @brief returns the json object of a list value
     *
     * @param encoded_value
     * @return json
     */
    json decode_list(const std::string &encoded_value, std::string::const_iterator &it);

    /**
     * @brief returns the json object of a dict value
     *
     * @param encoded_value
     * @return json
     */
    json decode_dict(const std::string &encoded_value, std::string::const_iterator &it);

    json decode_bencoded_value(const std::string &encoded_value, std::string::const_iterator &it);

public:
    /**
     * @brief  Decodes the bencoded value based on the type of the value
     *
     * @param encoded_value
     * @return json
     */
    json decode_bencoded_value(const std::string &encoded_value);
};
