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

inline log_level str_to_error_level(const string &s) {
    if(s.empty()) return level_critical;
    switch(s[0]) {
        case 'c': return level_critical;
        case 'e': return level_error;
        case 'i': return level_info;
        case 'w': return level_warning;
        case 'p': return level_profile;
        case 'd':
            if(s.size() < 7) return level_debug;
            switch(s[6]) {
                case 'h': return level_debug_hard;
                case 'm': return level_debug_mare;
            }
    }
    return level_critical;
}

}

#endif
