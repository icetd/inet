#include "WebSocketPacket.h"
#include "Buffer.h"
#include <arpa/inet.h>

using namespace inet;

void WebSocketPacket::decodeFrame(Buffer *frameBuf, Buffer *output)
{
    const char *msg = frameBuf->peek();

    int pos = 0;
    // get fin_
    fin_ = ((unsigned char)msg[pos] >> 7);
    // get opcode_
    opcode_ = msg[pos] & 0x0f;
    pos++;
    // get mask_
    mask_ = (unsigned char)msg[pos] >> 7;
    // get payload_length_
    payload_length_ = msg[pos] & 0x7f;
    pos++;
    if (payload_length_ == 126) {
        uint16_t length = 0;
        memcpy(&length, msg + pos, 2);
        pos += 2;
        payload_length_ = ntohs(length);
    } else if (payload_length_ == 127) {
        uint64_t length = 0;
        memcpy(&length, msg + pos, 8);
        pos += 8;
        payload_length_ = ntohl(length);
    }
    // get masking_key_
    if (mask_ == 1) {
        for (int i = 0; i < 4; i++)
            masking_key_[i] = msg[pos + i];
        pos += 4;
    }

    if (mask_ != 1) {
        output->append(msg + pos, payload_length_);
    } else {
        for (uint64_t i = 0; i < payload_length_; i++) {
            output->append(msg[pos + i] ^ masking_key_[i % 4], payload_length_);
        }
    }
}

void WebSocketPacket::encodeFrame(Buffer *output, Buffer *data) const
{
    uint8_t onebyte = 0;
    onebyte |= (fin_ << 7);
    onebyte |= (rsv1_ << 6);
    onebyte |= (rsv2_ << 5);
    onebyte |= (rsv3_ << 4);
    onebyte |= (opcode_ & 0x0F);
    output->append((char *)&onebyte, 1);

    // set mask flag
    onebyte = 0;
    onebyte = onebyte | (mask_ << 7);

    // set patload_length
    onebyte = 0;
    int length = data->readableBytes();

    if (length < 126) {
        onebyte |= length;
        output->append((char *)&onebyte, 1);
    } else if (length >= 126 && length <= 65535) {
        onebyte |= 126;
        output->append((char *)&onebyte, 1);
        
        uint16_t len16 = htons(length);
        output->append((char *)&len16, 2);
    } else if (length >= 65536) {
        onebyte |= 127;
        output->append((char *)&onebyte, 1);
        
        uint64_t len64 = htobe64(length);
        output->append((char *)&len64, 8);
    }

    if (mask_ == 1) {
        output->append((char *)masking_key_, 4); // save masking key

        char value = 0;
        for (uint64_t i = 0; i < payload_length_; ++i)
        {
            value = *(char *)(data->peek());
            data->retrieve(1);
            value = value ^ masking_key_[i % 4];
            output->append(&value, 1);
        }
    } else {
        output->append(data->peek(), data->readableBytes());
    }
}