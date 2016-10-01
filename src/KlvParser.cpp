//
//  KlvParser.cpp
//  libklv
//
//  Created by <author> on 17/08/2016.
//
//

#include "KlvParser.hpp"
#include <algorithm>
#include <stdio.h>

KlvParser::KlvParser() {
    this->state = STATE_INIT;
    this->ber_len = 0;
    this->val_len = 0;
    this->num_ber_len_bytes_read = 0;
    this->ber_long_form = false;
}

KlvParser::~KlvParser() {

}

/**
 * Parses a single byte and returns the parsed KLV (if successful). If partial
 * KLV (i.e. not enough bytes) then the parser is left in a "partial" state
 * where if this method is called again with the remaining bytes, this method
 * will then return the parsed KLV. If invalid KLV is parsed, then an exception
 * will be thrown and the return value will be NULL.
 *
 * @param  byte byte to parse
 * @return       the new kLV parsed from the input bytes. NULL if no KLV was
 *               parsed or error occured. Ownership is transfered to the caller.
 */
KLV* KlvParser::parseByte(uint8_t byte) {
    switch(state) {
    case STATE_INIT: {       // init state
        // keep parsing until last 16 bytes match with KLV universal key
        // keep parsing until first 4 bytes of the 16-byte KLV universal key is detected
        // store last 16 bytes in key
        // keep parsing until first 4 bytes of the 16-byte key match the 4-byte KLV unversal key header
        key.push_back(byte);

        if(key.size() > 16) {
            key.erase(key.begin());
        }

        // TODO: come up with faster optimized method
        if( (std::search(key.begin(), key.end(), 
            SMPTE_KLV_UL_HEADER, SMPTE_KLV_UL_HEADER+SMPTE_KLV_UL_HEADER_LEN)) == key.begin() 
            && key.size() == 16) {
                // found the 4-byte KLV universal key header at beginning
                state = STATE_KEY;
                printf("KlvParser transitioning to STATE_KEY\n");
        }
        break;
    }
    case STATE_KEY: {       // read KLV 16-byte universal key
        // read byte, and figure out if long or short BER form
        len.push_back(byte);
        ber_long_form = (bool) (byte & 0b10000000);

        if(ber_long_form) {
            state = STATE_LEN_HEADER;
            ber_len = byte & 0b01111111;
            val_len = 0;
            printf("BER-Len field is long-form\n");
            printf("BER len: %ld\n", ber_len);
            printf("KlvParser transitioning to STATE_LEN_HEADER\n");
        } else {
            state = STATE_LEN;
            val_len = byte & 0b01111111;
            printf("BER-Len field is short-form\n");
            printf("Value length: %ld\n", val_len);
            printf("KlvParser transitioning to STATE_LEN\n");
        }
        break;
    }
    case STATE_LEN_HEADER: { // read first byte in BER-encoded length field
        // keep parsing for ber_len bytes and store into val_len
        len.push_back(byte);
        val_len <<= 8;
        val_len |= byte;
        num_ber_len_bytes_read++;
        if(num_ber_len_bytes_read == ber_len) {
            state = STATE_LEN;
            printf("Value length: %ld\n", val_len);
            printf("KlvParser transitioning to STATE_LEN\n");
        }
        break;
    }
    case STATE_LEN: {       // read entire BER-encoded length field
        // keep parsing for val_len bytes and store into val
        val.push_back(byte);
        if(val.size() == val_len) {
            state = STATE_VALUE;
            printf("KlvParser transitioning to STATE_VALUE\n");
        } else {
            break;
        }
    }
    case STATE_VALUE: {      // read value field
        // construct KLV object

        // for right now this will only construct the "root" part of the incoming KLV
        // ...and not recurse into the tree if there is embedded KLV

        // TODO: use smart pointer here and transfer ownership to caller
        KLV *klv = new KLV(key, len, val);
        state = STATE_INIT;
        return klv;
    }
    default:
        // not supposed to be here :)
        break;
    }
    return NULL;
}


//
// /**
//  * Parses a byte deque and returns the parsed KLV (if successful). If partial
//  * KLV (i.e. not enough bytes) then the parser is left in a "partial" state
//  * where if this method is called again with the remaining bytes, this method
//  * will then return the parsed KLV. If invalid KLV is parsed, then an exception
//  * will be thrown and the return value will be NULL.
//  *
//  * A std::deque is used for the input to allow bytes to be "consumed" by this
//  * parser. The parser will only consume enough bytes it parses as the length
//  * field in the parsed KLV tells it to. That is, if there are 200 bytes available
//  * and the length field makes it consume only up to 180 bytes, the leftover 20
//  * bytes will be left in the deque to allow the caller to handle leftover
//  * bytes.
//  *
//  * @param  bytes byte deque containing KLV to be parsed
//  * @return       the new kLV parsed from the input bytes. NULL if no KLV was
//  *               parsed or error occured.
//  */
// KLV* KlvParser::parseBytes(std::deque<uint8_t> &bytes) {
//     // Decode key (Should be 16-byte key!)
//     std::vector<uint8_t>::const_iterator iter;
//     iter = std::search(rawData.begin(), rawData.end(),
//                                 SMPTE_KLV_UL_HEADER, SMPTE_KLV_UL_HEADER+SMPTE_KLV_UL_HEADER_LEN);
//
//     if(iter != rawData.end())   // We found KLV UL header
//     {
//         //this->value.clear();
//         this->resetFields();
//
//         // Decode key
//         //std::copy(iter, iter+SMPTE_KLV_UL_HEADER_LEN, this->key.begin());
//         //this->key.insert(iter, iter+SMPTE_KLV_UL_HEADER_LEN, this->key.begin());
//         this->key.assign(iter, iter+KLV_KEY_SIZE);
//
//         // Decode length (BER encoded)
//         iter += KLV_KEY_SIZE;
//
//         uint8_t berForm = *iter;   // Get MSB
//         berForm >>= 7;
//         if(berForm == 0)
//         {
//             // BER short form
//             // Next 7 bits tell us length
//             this->lenEncoded.push_back(*iter);
//             this->len = *iter;
//         }
//         else
//         {
//             // BER long form
//             // first byte (ignoring MSB) says how long len field is in bytes
//             uint8_t berLongFormLen = 0x7F & *iter;
//             this->lenEncoded.push_back(*iter);
//             iter++;
//             for(int i = 0; i < berLongFormLen; i++)
//             {
//                 this->lenEncoded.push_back(*iter);
//                 iter++;
//             }
//
//             // Get human-readable length
//             for(auto lenIter = this->lenEncoded.begin()+1; lenIter != this->lenEncoded.end(); lenIter++)
//             {
//                 this->len <<= 8;
//                 this->len |= *lenIter;
//             }
//         }
//
//         // Decode data
//         //std::copy(iter, iter+this->len, this->value.begin());
//         this->value.assign(iter, iter+this->len);
//         return true;
//     }
//     else
//     {
//         // Could not find valid KLV UL header in raw data
//         perror("ERROR: Invalid raw data, could not parse into valid KLV object!\n");
//         return false;
//     }
// }
