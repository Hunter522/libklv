//
//  KlvParser.hpp
//  libklv
//
//  Created by <author> on 17/08/2016.
//
//

#ifndef KlvParser_hpp
#define KlvParser_hpp

#include <cstdint>
#include <vector>
#include "Klv.h"

/**
 * KLV Parser
 */
class KlvParser {

public:

    /**
     * Encoding method for keys
     */
    enum KeyEncoding {
        KEY_ENCODING_1_BYTE,
        KEY_ENCODING_2_BYTE,
        KEY_ENCODING_4_BYTE,
        KEY_ENCODING_16_BYTE,
        KEY_ENCODING_BER_OID
    };


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
    KlvParser(std::vector<KeyEncoding> key_encodings);
    virtual ~KlvParser();

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
    virtual KLV* parseByte(uint8_t byte);

protected:
    bool checkIfContainsKlvKey(std::vector<uint8_t> data);
    void resetFields();

    /**
     * Parser state enum
     */
    enum State {
        STATE_INIT,       /// init state
        STATE_KEY,        /// read KLV 16-byte universal key
        STATE_LEN_HEADER, /// read first byte in BER-encoded length field
        STATE_LEN,        /// read entire BER-encoded length field
        STATE_VALUE       /// read value field
    };

    long                 ctr;             /// counter for bytes read by this parser
    State                state;           /// parser state
    std::vector<KeyEncoding> key_encodings; /// encoding to use for keys
    std::vector<uint8_t> key;             /// 16-byte universal key
    std::vector<uint8_t> len;             /// BER-encoded length
    std::vector<uint8_t> val;             /// value

    bool                 ber_long_form;   /// true if BER length field is long form, false if short form
    unsigned long        ber_len;         /// length of BER-encoded length field in bytes
    unsigned long        num_ber_len_bytes_read; /// number of bytes read for BER length field
    unsigned long        val_len;         /// length of value field in bytes
    
    KLV*                 parent;          /// parent KLV node, NULL if on top level branch
    KLV*                 child;           /// first child in branch, NULL if leave node
    KLV*                 previous_sibling;/// previous KLV node on branch, NULL if none. Typically if first node in branch, this will be NULL
    KLV*                 next_sibling;    /// next KLV node on branch, NULL if none
};


#endif /* KlvParser_hpp */
