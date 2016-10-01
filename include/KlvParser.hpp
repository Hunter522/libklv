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
    KlvParser();
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
    KLV* parseByte(uint8_t byte);

protected:

private:

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

    State          state;               /// parser state
    std::vector<uint8_t> key;            /// 16-byte universal key
    std::vector<uint8_t> len;           /// BER-encoded length
    std::vector<uint8_t> val;           /// value

    bool ber_long_form;                 /// true if BER length field is long form, false if short form
    unsigned long ber_len;              /// length of BER-encoded length field in bytes
    unsigned long num_ber_len_bytes_read; /// number of bytes read for BER length field
    unsigned long val_len;              /// length of value field in bytes
    
};


#endif /* KlvParser_hpp */
