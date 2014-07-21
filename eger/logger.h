#ifndef EGER_LOGGER_H
#define EGER_LOGGER_H

#include <thread>
#include <eger/types.h>
#include <assert.h>

namespace eger {

class writer;

class instance : public log_map {
    public:
    instance();
    ~instance();

    void start_writer();
    void start_sync_writer();

    static inline instance &get_instance() {
        assert(eger_instance_);
        return *eger_instance_;
    }

    inline bool is_using_this_level(log_level lvl) {
        return !operator[]((size_t) lvl).empty();
    }

    inline bool is_critical_level(log_level lvl) {
        return lvl == level_critical;
    }

    void pass_to_writer(log_stream *ls);

    private:
    instance(const instance &);
    instance(instance &&);

    public:
    size_t maximum_log_size;
    bool ansi_colors;

    private:

    static instance *eger_instance_;
    static writer *wrt;
    static std::thread *wrt_thread;
};

template<class lambda>
void logger(log_level log_lvl, const lambda &func) {
    if(!instance::get_instance().is_using_this_level(log_lvl))
        return;
    instance::get_instance().pass_to_writer(func());
}

inline bool is_using_this_level(log_level log_lvl) {
    return instance::get_instance().is_using_this_level(log_lvl);
}

#define log_func_header(log_level_m) \
    eger::logger(eger::log_level_m, [&] () -> eger::log_stream* { \
        eger::log_stream *new_stream = new eger::log_stream(eger::log_level_m); \
        *new_stream << 

#define log_func_footer \
        ; return new_stream; })

#define to_log(level, streaming_content) \
    log_func_header(level) streaming_content log_func_footer

#define to_log_multiline(level, streaming_content) \
    log_func_header(level) streaming_content ; new_stream->multiline = true log_func_footer

#define log_critical(streaming_content) to_log(level_critical, streaming_content)
#define log_error(streaming_content) to_log(level_error, streaming_content)
#define log_info(streaming_content) to_log(level_info, streaming_content)
#define log_warning(streaming_content) to_log(level_warning, streaming_content)
#define log_profile(streaming_content) to_log(level_profile, streaming_content)
#define log_profile_multiline(streaming_content) to_log_multiline(level_profile, streaming_content)
#define log_debug(streaming_content) to_log(level_debug, streaming_content)
#define log_debug_multiline(streaming_content) to_log_multiline(level_debug, streaming_content)

#ifdef NDEBUG
#define log_debug_hard(streaming_content)
#define log_debug_hard_multiline(streaming_content)

#define log_debug_mare(streaming_content)
#define log_debug_mare_multiline(streaming_content)
#else
#define log_debug_hard(streaming_content) to_log(level_debug_hard, streaming_content)
#define log_debug_hard_multiline(streaming_content) to_log_multiline(level_debug_hard, streaming_content)

#define log_debug_mare(streaming_content) to_log(level_debug_mare, streaming_content)
#define log_debug_mare_multiline(streaming_content) to_log_multiline(level_debug_mare, streaming_content)
#endif

}

#endif
