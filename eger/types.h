#ifndef EGER_TYPES_H
#define EGER_TYPES_H

#include <string>
#include <vector>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>

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
        moment(boost::posix_time::microsec_clock::local_time()),
        multiline(false)
    {}
    log_level lvl;
    boost::posix_time::ptime moment;
    bool multiline;
};

const size_t log_level_size = ((size_t) level_debug_mare) + 1;

}

#endif
