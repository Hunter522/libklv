#ifndef KLV_FORMAT_EXCEPTION_H
#define KLV_FORMAT_EXCEPTION_H

#include <stdexcept>
#include <cstdio>

class KlvFormatException : public std::exception {
public:
    KlvFormatException(char* reason) {
        this->reason = reason;
    }

    virtual const char* what() const throw() {
        return reason;
    }

private:
    char* reason;
};

#endif // KLV_FORMAT_EXCEPTION_H
