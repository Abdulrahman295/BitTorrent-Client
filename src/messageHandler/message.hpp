#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct Block
{
    uint32_t index;
    uint32_t begin;
    std::vector<uint8_t> data;
};

enum class MessageType
{
    CHOKE = 0,
    UNCHOKE = 1,
    INTERESTED = 2,
    NOT_INTERESTED = 3,
    HAVE = 4,
    BITFIELD = 5,
    REQUEST = 6,
    PIECE = 7,
    CANCEL = 8
};

class Message
{
private:
    MessageType type;
    uint32_t length;
    std::vector<uint8_t> payload;

public:
    /**
     * @brief Construct a new Message object
     *
     * @param type
     * @param length
     * @param payload
     */
    Message(MessageType type, uint32_t length, std::vector<uint8_t> payload);

    /**
     * @brief Get the message type
     *
     * @return MessageType
     */
    MessageType get_type();

    /**
     * @brief Get the length of the message
     *
     * @return size_t
     */
    uint32_t get_length();

    /**
     * @brief Get the payload field of the message
     *
     * @return std::vector<uint8_t>
     */
    std::vector<uint8_t> get_payload();

    /**
     * @brief Parse the payload of the piece message and return a block struct
     *
     * @return Block
     */
    Block get_block();
};