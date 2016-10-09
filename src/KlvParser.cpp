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

/**
 * Constructs a new KLV parser. Since keys can be encoded using different methods, 
 * a vector of KeyEncodings must be passed to allow the parser to know how to decode
 * keys correctly. A vector is used to allow different encodings for different levels
 * of KLV when parsing embedded KLV triplets. For example, the root/base KLV may use a
 * universal 16-byte key, and the embedded KLVs may use a local data set encoding of
 * BER-OID. If no KeyEncoding is specified, then it is assumed that 16-byte keys are
 * used at the root/base key and BER-OID encoding is used for the embedded KLVs.
 * 
 * @param key_encodings vector of KeyEncodings to be used when parsing
 */
KlvParser::KlvParser(std::vector<KeyEncoding> key_encodings) {
    ctr = 0;
    resetFields();
    this->key_encodings = key_encodings;
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
    ctr++;
    printf("PARSING BYTE %ld : %x\n", ctr, byte);
    switch(state) {
    case STATE_INIT: {       // init state
        // keep parsing until last 16 bytes match with KLV universal key
        // keep parsing until first 4 bytes of the 16-byte KLV universal key is detected
        // store last 16 bytes in key
        // keep parsing until first 4 bytes of the 16-byte key match the 4-byte KLV unversal key header
        key.push_back(byte);

        // handle looking for KEY depending on how its encoded
        switch(key_encodings[0]) {
        case KEY_ENCODING_1_BYTE: {
            state = STATE_KEY;
            printf("KlvParser transitioning to STATE_KEY\n");
            break;
        }
        case KEY_ENCODING_2_BYTE: {
            if(key.size() == 2) {
                state = STATE_KEY;
                printf("KlvParser transitioning to STATE_KEY\n");
            }
            break;
        }
        case KEY_ENCODING_4_BYTE: {
            if(key.size() == 4) { 
                state = STATE_KEY;
                printf("KlvParser transitioning to STATE_KEY\n");
            }
            break;
        }
        case KEY_ENCODING_16_BYTE: {
            if(key.size() > 16)
                key.erase(key.begin());

            if(checkIfContainsKlvKey(key)) {
                // found the 4-byte KLV universal key header at beginning
                state = STATE_KEY;
                printf("KlvParser transitioning to STATE_KEY\n");
            }
            break;
        }
        case KEY_ENCODING_BER_OID: {
            // keep parsing until we read a byte where bit 8 is 0 (indicating we've read the LSB of the BER-OID key)
            // TODO: the KLV class actually should have a human-readable tag due to this encoding technique
            if(!(byte & 0b10000000)) {
                state = STATE_KEY;
                printf("KlvParser transitioning to STATE_KEY\n");
            }
            break;
        }
        default:
            // not supposed to be here :-)
            break;
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
        // TODO: use smart pointer here and transfer ownership to caller
        KLV *klv = new KLV(key, len, val);


        // for right now this will only construct the "root" part of the incoming KLV
        // ...and not recurse into the tree if there is embedded KLV
        // might have to do setters for KLV

        // TODO: implement embedded KLV parsing
        // could do this by declaring a KlvParser on the stack as a local variable, then going through all of the bytes
        // in the value field. If we get a KLV back, then we know to add it to
        // the tree of the KLV. this will require KLV to have setters for the tree fields
        // by doing this, we "reset" the parser state variables by using a completely different parser...makes sense
        // so this class also will construct the tree...which means we'll need some fields to temp hold that info
        
        // iterate through each byte in the value field
        // store into local vector
        // check if last 16 bytes are KLV key
        // if so then create a sub_klv_parser and then start parsing using those 16 bytes then the next bytes coming in

        printf("key_encodings.size() : %ld\n", key_encodings.size());
        // key_encodings.erase(key_encodings.begin());
        if(key_encodings.size() > 1) {
            printf("Creating sub_klv_parser...\n");
            KlvParser sub_klv_parser(std::vector<KeyEncoding>(key_encodings.begin()+1, key_encodings.end()));
            std::vector<KLV*> sub_klvs;
            KLV* sub_klv = NULL;
            int i = 0;

            // parse the sub-KLVs
            for(i = 0; i < val.size(); i++) {
                sub_klv = sub_klv_parser.parseByte(val[i]);
                if(sub_klv != NULL) {
                    printf("new sub KLV\n");
                    sub_klvs.push_back(sub_klv);
                    sub_klv = NULL;
                }
            }

            // assign child of THIS klv to the first child in the vector
            klv->setChild(sub_klvs[0]);

            // assign the next and previous sibling fields and the parent field in each of the sub_klvs
            for(i = 0; i < sub_klvs.size(); i++) {
                if(sub_klvs.size() > 1) {
                    if(i == 0) {
                        // beginning
                        sub_klvs[i]->setNextSibling(sub_klvs[i+1]);
                    } else if(i == sub_klvs.size()) {
                        // end
                        sub_klvs[i]->setPreviousSibling(sub_klvs[i-1]);
                    } else {
                        sub_klvs[i]->setNextSibling(sub_klvs[i+1]);
                        sub_klvs[i]->setPreviousSibling(sub_klvs[i-1]);
                    }
                }
                sub_klvs[i]->setParent(klv);
            }
        }
        
        // reset state machine back to STATE_INIT and return parsed KLV
        resetFields();
        return klv;
    }

    default:
        // not supposed to be here :)
        break;
    } // end switch(state)

    return NULL;
}

bool KlvParser::checkIfContainsKlvKey(std::vector<uint8_t> data) {
    // TODO: come up with faster optimized method
    return ((std::search(data.begin(), data.end(), 
            SMPTE_KLV_UL_HEADER, SMPTE_KLV_UL_HEADER+SMPTE_KLV_UL_HEADER_LEN)) == data.begin() 
            && data.size() == 16);
}

void KlvParser::resetFields() {
    state = STATE_INIT;
    ber_len = 0;
    val_len = 0;
    num_ber_len_bytes_read = 0;
    ber_long_form = false;

    key.clear();
    len.clear();
    val.clear();

    child = NULL;
    parent = NULL;
    next_sibling = NULL;
    previous_sibling = NULL;
}