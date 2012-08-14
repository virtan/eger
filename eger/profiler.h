#ifndef EGER_PROFILER_H
#define EGER_PROFILER_H

#include <iomanip>
#include <eger/timer.h>
#include <eger/named_instance.h>
#include <eger/logger.h>
#include <eger/formatter.h>

namespace eger {

#define profiler_start(named_inst) \
    (eger::is_using_this_level(eger::level_profile) ? \
        named_instance(eger::timer, named_inst).start(), 0 : 0)

#define profiler_stop(named_inst) \
    (eger::is_using_this_level(eger::level_profile) ? \
        named_instance(eger::timer, named_inst).stop(), 0 : 0)

#define profiler_reset(named_inst) \
    (eger::is_using_this_level(eger::level_profile) ? \
        named_instance(eger::timer, named_inst).reset(), 0 : 0)

#define profiler_dump(named_inst) \
    (eger::is_using_this_level(eger::level_profile) ? \
        log_profile_multiline(eger::human_readable_number( \
            ((double) (named_instance(eger::timer, named_inst).total()) / 1000000)) << \
            "s\t" << #named_inst), \
        named_instance(eger::timer, named_inst).reset(), 0 : 0)

#define profiler_dump_2(named_inst, extra) \
    (eger::is_using_this_level(eger::level_profile) ? \
        log_profile_multiline(eger::human_readable_number( \
            ((double) (named_instance(eger::timer, named_inst).total()) / 1000000)) << \
            "s\t" << #named_inst << " (" << extra << ")"), \
        named_instance(eger::timer, named_inst).reset(), 0 : 0)

}

#endif
