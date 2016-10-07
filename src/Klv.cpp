#include "Klv.h"
#include <iostream>
#include <algorithm>
#include <string>

/**
 * @brief Convinience constructor to create a KLV object with a specified universal key
 *        and data buffer. This will also encode the length (BER).
 *
 * @param key 16-byte global unique identifier
 * @param val data buffer
 */
KLV::KLV(const std::vector<uint8_t> key, const std::vector<uint8_t> val) {
    this->key = key;
    this->value = val;

    // TODO: calc BER encoded length and store it in lenEncoded field



    // TODO: figure out BER length
    // TODO: figure out human-readable len
}

/**
 * @brief Constructs a KLV object from given key, length, and value
 *
 * @param key 16-byte global unique identifier
 * @param len BER-encoded length
 * @param val data buffer
 */
KLV::KLV(const std::vector<uint8_t> key, const std::vector<uint8_t> len, const std::vector<uint8_t> val) {
    this->key = key;
    this->len_encoded = len;
    this->value = val;
    this->parent = NULL;
    this->child = NULL;
    this->next_sibling = NULL;
    this->previous_sibling = NULL;

    // BER encoding has a short form and long form
    // Most significant bit (bit 7) is the short/long form flag
    // 0 - short form
    // 1 - long form

    // BER Short Form Length encoding
    //
    // example short form BER length:
    //     [ 0 1 0 0 1 1 0 0 ]
    // bit   7 6 5 4 3 2 1 0
    // Length field using the short form are represented using a single byte (8
    // bits). The most significant bit in this byte signals that the long form is being used. The last
    // seven bits depict the number of bytes that follow the BER encoded length.
    // Short form BER encoding is form value fields less than 128 bytes in length

    // BER Long Form Length encoding
    //
    // example long form BER length:
    //     [1 0 0 0 0 0 0 1  1 1 0 0 1 0 0 1]
    // bit  7 6 5 4 3 2 1 0  7 6 5 4 3 2 1 0
    // The long form encodes length field using multiple bytes. The first byte
    // indicates long form encoding as well as the number of subsequent bytes that represent the length.
    // The bytes that follow the leading byte are the encoding of an unsigned binary integer equal to the
    // number of bytes in the packet.
    // Long form BER encoding is for value fields more than 128 bytes in length

    if(len.size() > 1) {
        // BER long form

        // get the length of the BER field in bytes
        this->ber_len = len[0] & 0b01111111;

        if(ber_len != len.size() - 1) {
            throw std::invalid_argument("len argument encoded length " + std::to_string(ber_len) + " did not match actual length " + std::to_string(len.size() - 1));
        }

        // iterate through the len vector for that many bytes
        this->len = len[1];
        for(int i = 1; i < ber_len; i++) {
            this->len <<= 8;
            this->len |= len[2+i];
        }

    } else {
        // BER short form
        this->len = len[0];
    }
}

/**
 * @brief Destructor
 */
KLV::~KLV() {
    // this class will not delete it's parent, children, or siblings
    // the external owner of this class must handle that
}

/**
 * @brief Returns fully encoded raw data of this KLV.
 *
 * @return Vector of encoded bytes. Empty vector if key, len, or value field has not been set.
 */
std::vector<uint8_t> KLV::toBytes() {
    std::vector<uint8_t> buffer;
    if(key.size() > 0 && len_encoded.size() > 0 && value.size() > 0) {
        // append Key, Len, and Value to buffer
        buffer.insert(buffer.end(), key.begin(), key.end());
        buffer.insert(buffer.end(), len_encoded.begin(), len_encoded.end());
        buffer.insert(buffer.end(), value.begin(), value.end());
    }
    return buffer;
}
