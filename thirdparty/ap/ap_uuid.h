#ifndef AP_UUID_H
#define AP_UUID_H
#include <string>
/*
    Astral Pixel uuid generation utility

    --- Usage ---
        In one of your .c/.cpp files define this to create the implementation functions
        #define AP_UUID_H
        #include "ap_uuid.h"

        Use uuid_v4() to generate a new uuid
*/


namespace ap {

// 128-bit basic UUID type that allows comparison and sorting.
struct uuid {
    std::uint_fast64_t ab = 0;
    std::uint_fast64_t cd = 0;
    bool operator==(const uuid& other) const;
    bool operator!=(const uuid& other) const;
    bool operator <(const uuid& other) const;
};


// Generate and uuid version 4,  anonymous, fast; con: uuids "can clash"
uuid uuid_v4();

// outputs a string from an uuid with this format: "f81d4fae-7dec-11d0-a765-00a0c91e6bf6"
std::string uuid_to_string(uuid u);

// creates an uui from a string with this format: "f81d4fae-7dec-11d0-a765-00a0c91e6bf6"
uuid uuid_from(const std::string& uustr);

// creates an uui from two numbers
uuid uuid_from_nums(std::uint_fast64_t ab, std::uint_fast64_t cd);

}

namespace std {
    template<>
    struct hash< ap::uuid > {
    public:
        // hash functor: hash uuid to size_t value by pseudorandomizing transform
        size_t operator()(const ap::uuid& uuid) const {
            if constexpr(sizeof(size_t) > 4) {
                return size_t(uuid.ab ^ uuid.cd);
            } else {
                std::uint_fast64_t hash64 = uuid.ab ^ uuid.cd;
                return size_t(std::uint_fast32_t(hash64 >> 32) ^ std::uint_fast32_t(hash64));
            }
        }
    };
}


#ifdef AP_UUID_IMPL
#include <random>
#include <sstream>
#include <iomanip>

namespace ap {

uuid uuid_v4() {
    static std::random_device rd;
    static std::uniform_int_distribution<std::uint_fast64_t> dist(0, (std::uint_fast64_t)(~0));
    uuid my;
    my.ab = dist(rd);
    my.cd = dist(rd);
    my.ab = (my.ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    my.cd = (my.cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    return my;
}

bool uuid::operator==(const ap::uuid& other) const {
    return ab == other.ab && cd == other.cd;
}

bool uuid::operator!=(const ap::uuid& other) const {
    return !operator==(other);
}

bool uuid::operator<(const ap::uuid& other) const {
    if (ab < other.ab) return true;
    if (ab > other.ab) return false;
    if (cd < other.cd) return true;
    return false;
}

uuid uuid_from_nums(std::uint_fast64_t ab, std::uint_fast64_t cd) {
    uuid u;
    u.ab = ab; u.cd = cd;
    return u;
}

uuid uuid_from(const std::string& uustr) {
    char sep;
    std::uint_fast64_t a, b, c, d, e;
    uuid u = { 0, 0 };
    auto idx = uustr.find_first_of("-");
    if (idx != std::string::npos) {
        // classic hex notation
        std::stringstream ss(uustr);
        if (ss >> std::hex >> a >> sep >> b >> sep >> c >> sep >> d >> sep >> e) {
            if (ss.eof()) {
                u.ab = (a << 32) | (b << 16) | c;
                u.cd = (d << 48) | e;
            }
        }
        
    }
    return u;
}


std::string uuid_to_string(uuid u) {
    std::stringstream ss;
    ss << std::hex << std::nouppercase << std::setfill('0');

    uint32_t a = (u.ab >> 32);
    uint32_t b = (u.ab & 0xFFFFFFFF);
    uint32_t c = (u.cd >> 32);
    uint32_t d = (u.cd & 0xFFFFFFFF);

    ss << std::setw(8) << (a) << '-';
    ss << std::setw(4) << (b >> 16) << '-';
    ss << std::setw(4) << (b & 0xFFFF) << '-';
    ss << std::setw(4) << (c >> 16) << '-';
    ss << std::setw(4) << (c & 0xFFFF);
    ss << std::setw(8) << d;

    return ss.str();
}


}

#endif
#endif // AP_UUID_H

