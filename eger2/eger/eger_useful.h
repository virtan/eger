#ifndef EGER_USEFUL_H
#define EGER_USEFUL_H

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#endif
