#include <cstdint>
#include <vector>
#include <arpa/inet.h>

#include "messageHandler/message.hpp"
#include <stdexcept>

Message::Message(MessageType type, uint32_t length, std::vector<uint8_t> payload)
{
    this->type = type;
    this->length = length;
    this->payload = payload;
}

MessageType Message::get_type()
{
    return this->type;
}

uint32_t Message::get_length()
{
    return this->length;
}

std::vector<uint8_t> Message::get_payload()
{
    return this->payload;
}

Block Message::get_block()
{
    Block result;

    if (this->type != MessageType::PIECE)
    {
        throw std::runtime_error("Not a PIECE message");
    }

    // payload is of the form: <index><begin><block>
    constexpr uint32_t INDEX_OFFSET = 0;
    constexpr uint32_t BEGIN_OFFSET = 4;
    constexpr uint32_t DATA_OFFSET = 8;

    if (this->payload.size() < DATA_OFFSET)
    {
        throw std::runtime_error("Piece message payload too short");
    }

    result.index = ntohl(*reinterpret_cast<const uint32_t *>(this->payload.data() + INDEX_OFFSET));

    result.begin = ntohl(*reinterpret_cast<const uint32_t *>(this->payload.data() + BEGIN_OFFSET));

    result.data = std::vector<uint8_t>(this->payload.begin() + DATA_OFFSET, this->payload.end());

    return result;
}