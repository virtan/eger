#ifndef EGER_USEFUL_H
#define EGER_USEFUL_H

#include <sstream>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

enum a_switch : bool {
    disabled = false,
    enabled = true
};

bool autodetect_ansi(int fd);

std::ostringstream &current_time_to_stream(std::ostringstream &out);

bool writev_all(int fd, const struct iovec *iov, int iovcnt);

// returns current time in microseconds
size_t now_us();

void lower_this_thread_priority();

std::string human_readable_number(double d);

#endif
