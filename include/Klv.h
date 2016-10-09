#ifndef KLV_H
#define KLV_H

#include <stdint.h>
#include <vector>
#include <unordered_map>

// Is this necessary? No rule saying KLV has a max payload size...
#define MAX_KLV_VALUE_SIZE 4294967296
#define KLV_KEY_SIZE 16

// Key definitions
#define SMPTE_KLV_UL_HEADER_LEN 4
const uint8_t SMPTE_KLV_UL_HEADER[] = {0x06, 0x0E, 0x2B, 0x34};   // SMPTE KLV Universal Label (UL) Header
                                                                  // SMPTE KLV headers will ALWAYS start with this

/**
 * @brief Simple class representing a Key-Length-Value (KLV).
 *
 * Only universal keys (16-bytes long) are supported.
 * Data lengths are Basic-Encoding-Rules (BER) encoded.
 *
 * This class represents KLV in two ways.
 * 1. As a simple data structure where the Key, Length, and Value can be accessed
 *    as std::vector<uint8_t> member variables
 * 2. As a tree, since the value field in the KLV may have sub-KLV items.
 *
 * A Map can be used to index each KLV with their key
 *
 * References:
 *   SMPTE 336-2007
 *   ST 0601.8          -   UAS Datalink Local Metadata Set
 */
class KLV {

public:
    KLV() {}
    KLV(const std::vector<uint8_t> key, const std::vector<uint8_t> val);
    KLV(const std::vector<uint8_t> key, const std::vector<uint8_t> len, const std::vector<uint8_t> val);
    virtual ~KLV();

    std::vector<uint8_t> getKey() const { return this->key; }
    std::vector<uint8_t> getLenEncoded() const { return this->len_encoded; }
    std::vector<uint8_t> getValue() const { return this->value; }
    unsigned long getLen() const { return this->len; }
    unsigned long getBerLen() const { return ber_len; }

    KLV* getParent() const { return this->parent; }
    KLV* getChild() const { return this->child; }
    KLV* getPrevious() const { return this->previous_sibling; }
    KLV* getNext() const { return this->next_sibling; }

    void setParent(KLV* parent) { this->parent = parent; }
    void setChild(KLV* child) { this->child = child; }
    void setPreviousSibling(KLV* previous) { this->previous_sibling = previous; }
    void setNextSibling(KLV* next) { this->next_sibling = next; }

    std::vector<uint8_t> toBytes();
    std::unordered_map<std::vector<uint8_t>, KLV> indexToMap();
    void addToMap(std::unordered_map<std::vector<uint8_t>, KLV> &map); 

    // operator overloads
    bool operator==(const KLV &other) const { 
        return (key == other.key
            && len_encoded == other.len_encoded
            && value == other.value
            && parent == other.parent
            && child == other.child
            && previous_sibling == other.previous_sibling
            && next_sibling == other.next_sibling);
    }

    // hashing functor
    struct hash {
        std::size_t operator()(const KLV& k) const {
            using std::hash;

            // unchecked
            // {
            //     int hash = 17;
            //     hash = hash * 31 + firstField.GetHashCode();
            //     hash = hash * 31 + secondField.GetHashCode();
            //     return hash;
            // }
            int res = 17;
            for(auto b : k.key)
                res = res * 31 + hash<uint8_t>()(b);   
            for(auto b : k.len_encoded)
                res = res * 31 + hash<uint8_t>()(b);   
            for(auto b : k.value)
                res = res * 31 + hash<uint8_t>()(b);   
            res = res * 31 + hash<unsigned long>()(k.len);
            res = res * 31 + hash<unsigned long>()(k.ber_len);
            return res;
        }
    };

private:
    std::vector<uint8_t> key;             /// Key (1,2,4, or 16 bytes in length) (typically 16-byte universal key or BER-OID for LDS tags)
    std::vector<uint8_t> len_encoded;     /// Data length (BER) (short & long form)
    std::vector<uint8_t> value;           /// Value (variable-length)
    unsigned long        len;             /// Data length in human-readable format
    unsigned long        ber_len;         /// Length of BER len field
    KLV*                 parent;          /// parent KLV node, NULL if on top level branch
    KLV*                 child;           /// first child in branch, NULL if leave node
    KLV*                 previous_sibling;/// previous KLV node on branch, NULL if none. Typically if first node in branch, this will be NULL
    KLV*                 next_sibling;    /// next KLV node on branch, NULL if none
};


// specialize the hash function for std::vector<uint8_t> so that it may be used as a key in STL maps
namespace std {
    template <>
    struct hash<std::vector<uint8_t>> {
        std::size_t operator()(const std::vector<uint8_t>& k) const {
            int res = 17;
            for(auto b : k)
                res = res * 31 + std::hash<uint8_t>()(b);
            return res;
        }
    };
}


#endif // KLV_H
