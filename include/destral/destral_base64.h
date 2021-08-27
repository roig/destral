#pragma once

#include <string>
#include <cassert>

namespace ds {
    namespace detail {

    // Implementation similar to Apache base64 implementation
    static const unsigned char prtosix[256] =
    {
        /* ASCII table */
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    static inline std::size_t base64_decode_len(const char* bufcoded) {
        assert(bufcoded);

        std::size_t nbytesdecoded;
        const unsigned char* bufin;
        std::size_t nprbytes;

        bufin = (const unsigned char*)bufcoded;
        while (prtosix[*(bufin++)] <= 63);

        nprbytes = (bufin - (const unsigned char*)bufcoded) - 1;
        nbytesdecoded = ((nprbytes + 3) / 4) * 3;

        return nbytesdecoded + 1;
    }

    static inline std::size_t base64_decode(char* bufplain, const char* bufcoded) {
        assert(bufplain);
        assert(bufcoded);

        std::size_t nbytesdecoded;
        const unsigned char* bufin;
        unsigned char* bufout;
        std::size_t nprbytes;

        bufin = (const unsigned char*)bufcoded;
        while (prtosix[*(bufin++)] <= 63);
        nprbytes = (bufin - (const unsigned char*)bufcoded) - 1;
        nbytesdecoded = ((nprbytes + 3) / 4) * 3;

        bufout = (unsigned char*)bufplain;
        bufin = (const unsigned char*)bufcoded;

        while (nprbytes > 4) {
            *(bufout++) =
                (unsigned char)(prtosix[*bufin] << 2 | prtosix[bufin[1]] >> 4);
            *(bufout++) =
                (unsigned char)(prtosix[bufin[1]] << 4 | prtosix[bufin[2]] >> 2);
            *(bufout++) =
                (unsigned char)(prtosix[bufin[2]] << 6 | prtosix[bufin[3]]);
            bufin += 4;
            nprbytes -= 4;
        }

        /* Note: (nprbytes == 1) would be an error, so just ingore that case */
        if (nprbytes > 1) {
            *(bufout++) =
                (unsigned char)(prtosix[*bufin] << 2 | prtosix[bufin[1]] >> 4);
        }
        if (nprbytes > 2) {
            *(bufout++) =
                (unsigned char)(prtosix[bufin[1]] << 4 | prtosix[bufin[2]] >> 2);
        }
        if (nprbytes > 3) {
            *(bufout++) =
                (unsigned char)(prtosix[bufin[2]] << 6 | prtosix[bufin[3]]);
        }

        *(bufout++) = '\0';
        nbytesdecoded -= (4 - nprbytes) & 3;
        return nbytesdecoded;
    }

    static const char basis_64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    static inline std::size_t base64_encode_len(std::size_t len) {
        return ((len + 2) / 3 * 4) + 1;
    }

    static inline std::size_t base64_encode(char* encoded, const char* string, std::size_t len) {
        assert(string);
        assert(len);
        assert(encoded);

        size_t i;
        char* p;

        p = encoded;
        for (i = 0; i < len - 2; i += 3) {
            *p++ = basis_64[(string[i] >> 2) & 0x3F];
            *p++ = basis_64[((string[i] & 0x3) << 4) |
                ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = basis_64[((string[i + 1] & 0xF) << 2) |
                ((int)(string[i + 2] & 0xC0) >> 6)];
            *p++ = basis_64[string[i + 2] & 0x3F];
        }
        if (i < len) {
            *p++ = basis_64[(string[i] >> 2) & 0x3F];
            if (i == (len - 1)) {
                *p++ = basis_64[((string[i] & 0x3) << 4)];
                *p++ = '=';
            } else {
                *p++ = basis_64[((string[i] & 0x3) << 4) |
                    ((int)(string[i + 1] & 0xF0) >> 4)];
                *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
            }
            *p++ = '=';
        }

        *p++ = '\0';
        return p - encoded;
    }
    }


    // Expects a null terminated string
    static inline std::string base64_decode(const char* data) {
        assert(data);
        
        using namespace detail;
        std::string out;
        out.resize(base64_decode_len(data));
        size_t actualSize = base64_decode(&out[0], data);
        out.resize(actualSize);
        return out;
    }

    // Expects a non null terminated data array
    static inline std::string base64_encode(const char* data, std::size_t size) {
        assert(data);
        assert(size > 0);

        if (data == nullptr || size == 0) {

            return {};
        }

        using namespace detail;
        std::string out;
        out.resize(base64_encode_len(size));
        base64_encode(&out[0], data, size);

        // This will remove the null termination character
        out.pop_back();
        return out;
    }




}