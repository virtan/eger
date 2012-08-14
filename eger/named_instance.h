#ifndef EGER_NAMED_INSTANCE_H
#define EGER_NAMED_INSTANCE_H

#include <stdint.h>

namespace eger {

constexpr uint64_t compile_key_part(const char* s, uint64_t i, uint64_t c, uint64_t curr) {
    return curr * 0x100 +
        (c == 0 ? s[sizeof(uint64_t) * i] :
            eger::compile_key_part(s, i, c - 1, s[sizeof(uint64_t) * i + c]));
}

template <uint64_t n>
constexpr uint64_t compile_key(const char (&s)[n], uint64_t i) {
    return n <= sizeof(uint64_t) * i ? 0 :
        (eger::compile_key_part(s, i, n < sizeof(uint64_t) * (i + 1) ?
                          n - sizeof(uint64_t) * i - 1 : sizeof(uint64_t) - 1, 0));
}

template <class type, uint64_t ... keys>
struct named_instance_storage {
    static type& value() {
        static type v;
        return v;
    }
};

#define named_instance(type, name) \
    (eger::named_instance_storage<type, \
        eger::compile_key(#name, 0), eger::compile_key(#name, 1), eger::compile_key(#name, 2), \
        eger::compile_key(#name, 3), eger::compile_key(#name, 4), eger::compile_key(#name, 5), \
        eger::compile_key(#name, 6), eger::compile_key(#name, 7), eger::compile_key(#name, 8), \
        eger::compile_key(#name, 9), eger::compile_key(#name,10), eger::compile_key(#name,11), \
        eger::compile_key(#name,12), eger::compile_key(#name,13), eger::compile_key(#name,14), \
        eger::compile_key(#name,15), eger::compile_key(#name,16), eger::compile_key(#name,17), \
        eger::compile_key(#name,18), eger::compile_key(#name,19), eger::compile_key(#name,20), \
        eger::compile_key(#name,21), eger::compile_key(#name,22), eger::compile_key(#name,23), \
        eger::compile_key(#name,24), eger::compile_key(#name,25), eger::compile_key(#name,26), \
        eger::compile_key(#name,27), eger::compile_key(#name,28), eger::compile_key(#name,29), \
        eger::compile_key(#name,30), eger::compile_key(#name,31) \
    >::value())

}

#endif
