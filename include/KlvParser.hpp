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
     *               parsed or error occured.
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

    State          state;   /// parser state

    // current parser state fields
    std::vector<uint8_t> key;   /// 16-byte universal key
    unsigned long ber_len;  /// length of BER-encoded length field in bytes
    unsigned long val_len;  /// length of value field in bytesd
};


#endif /* KlvParser_hpp */
