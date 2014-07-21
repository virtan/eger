#ifndef EGER_TYPES_H
#define EGER_TYPES_H

#include <string>
#include <vector>
#include <sstream>
#include <chrono>

namespace eger {

typedef std::string string;
typedef std::vector<string> log_map;

enum log_level {
        level_critical = 0,
        level_error,
        level_info,
        level_warning,
        level_profile,
        level_debug,
        level_debug_hard,
        level_debug_mare
};

struct log_stream : public std::ostringstream {
    log_stream(log_level _lvl) :
        lvl(_lvl),
        moment(std::chrono::system_clock::now()),
        multiline(false)
    {}
    log_level lvl;
    std::chrono::system_clock::time_point moment;
    bool multiline;
};

const size_t log_level_size = ((size_t) level_debug_mare) + 1;

}

#endif
