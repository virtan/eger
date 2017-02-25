#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <array>
#include <vector>
#include <thread>
#include <regex>
#include <sstream>
#include <iostream>

#include <eger/useful.h>
#include <eger/many_to_many_circular_queue.h>

// Typical logger call:
// log_warning("a warning number " << 5)

namespace eger {

    class logger_level {
        public:
            enum level : uint8_t {
                logger_level_critical = 0,
                logger_level_error,
                logger_level_warning,
                logger_level_info,
                logger_level_profile,
                logger_level_debug,
                logger_level_debug_hard,
                logger_level_debug_mare,
                logger_levels // size of level
            };

            enum destination : uint8_t {
                stderr,
                stdout,
                file,
                devnull
            };

            enum ansi_colors_switch : bool {
                no_ansi_colors = false,
                with_ansi_colors = true
            };

            enum location_switch : bool {
                no_location = false,
                with_location = true
            };

            struct level_data {
                level_data() :
                    enabled(false),
                    ansi_colors(false),
                    location(false),
                    dest(stderr),
                    path{0},
                    fd(-1),
                    number_of_writes_since_last_reopen(0),
                    last_reopen(0), // TODO: fast time
                    error_reported(false)
                {}

                bool enabled;
                bool ansi_colors;
                bool location;
                destination dest;
                char path[256];
                int fd;
                size_t number_of_writes_since_last_reopen;
                size_t last_reopen;
                bool error_reported;
                std::vector<struct iovec> scheduled_for_writing;
            };

            mode_t log_file_mask = S_IWGRP | S_IWOTH;

            // reopen log files every 1000 writes
            size_t max_number_of_writes_since_last_reopen = 1000;

            // reopen log files every 30 seconds
            size_t max_open_time_us = 30 * 1000000;

        private:
            std::array<level_data, logger_levels> levels;
            static bool autodetected_ansi_colors;

        public:

            // by default we're printing critical, error and warning only
            // we print them to stderr, ansi colors are enabled
            logger_level() {
                for(level i : {logger_level_critical, logger_level_error, logger_level_warning}) {
                    auto &ld = levels[i];
                    ld.enabled = true;
                    ld.ansi_colors = autodetected_ansi_colors;
                }
            }
            
            ~logger_level() {
                for(size_t i = 0; i < logger_levels; ++i)
                    levels[i].enabled = false;
            }

            // sets a loglevel details
            // examples:
            //     set(logger_level_warning, disabled);
            //     set(logger_level_error, enabled, with_ansi_colors, no_location,  file, "/tmp/log");
            //     set(logger_level_critical, enabled, no_ansi_colors, with_location, stderr);
            void set(level l, bool enabled, bool ansi_colors = autodetected_ansi_colors,
                    bool location = false, destination dest = stderr,
                    const std::string &path = std::string()) {
                if(enabled) {
                    auto &ld = levels[l];
                    ld.enabled = false;
                    ld.ansi_colors = ansi_colors;
                    ld.location = location;
                    ld.dest = devnull;
                    if(dest == file) {
                        ld.path[0] = 0;
                        path.copy(&ld.path[1], std::min(path.size() - 1, sizeof(ld.path) - 2), 1);
                        ld.path[sizeof(ld.path) - 1] = 0;
                        ld.path[0] = path[0];
                    }
                    ld.dest = dest;
                    ld.error_reported = false;
                    ld.enabled = enabled;
                } else {
                    levels[l].enabled = enabled;
                }
            }

            // for console and config use
            // example:
            //     logger.level_warning = disabled
            //     logger.level_error = enabled, with_ansi_colors, file "/tmp/log"
            //     logger.level_critical = enabled, no_ansi_colors, stderr
            //     logger.level_info = "/some/file"
            void set(level l, const std::string &settings) {
                static std::regex re("(\"[^\"]*\")|enabled|disabled|with_ansi_colors|"
                        "no_ansi_colors|with_location|no_location|stderr|stdout|devnull|file");
                bool enabled = false;
                bool ansi_colors = autodetected_ansi_colors;
                bool location = false;
                destination dest = stderr;
                std::string path;
                std::sregex_token_iterator it(settings.begin(), settings.end(), re);
                std::sregex_token_iterator reg_end;
                for(; it != reg_end; ++it) {
                    const auto &rer = it->str();
                    if(rer == "enabled") enabled = true;
                    else if(rer == "disabled") enabled = false;
                    else if(rer == "with_ansi_colors") { ansi_colors = true; enabled = true; }
                    else if(rer == "no_ansi_colors") { ansi_colors = false; enabled = true; }
                    else if(rer == "with_location") { location = true; enabled = true; }
                    else if(rer == "no_location") { location = false; enabled = true; }
                    else if(rer == "stderr") { dest = stderr; enabled = true; }
                    else if(rer == "stdout") { dest = stdout; enabled = true; }
                    else if(rer == "devnull") { dest = devnull; enabled = true; }
                    else if(rer == "file") { dest = file; enabled = true; }
                    else if(rer[0] == '"') {
                        dest = file;
                        path.assign(rer, 1, rer.size() - 2);
                        enabled = true;
                    }
                }
                if(dest == file && path.empty()) dest = devnull;
                set(l, enabled, ansi_colors, location, dest, path);
            }

            inline bool is_enabled(level l) const {
                return levels[l].enabled;
            }

            level_data &level_details(level l) {
                return levels[l];
            }

            void write_reports() {
                for(auto &ld : levels) {
                    if(likely(!ld.enabled || ld.dest == devnull)) {
                        if(unlikely(ld.fd != -1)) {
                            if(ld.fd > 2) close(ld.fd);
                            ld.fd = -1;
                        }
                    } else {
                        if(unlikely(needs_reopen(ld))) {
                            if(ld.dest == file) close(ld.fd);
                            ld.fd = -1;
                        }
                        if(unlikely(ld.fd == -1)) {
                            switch(ld.dest) {
                                case stdout: ld.fd = 1; break;
                                case stderr: ld.fd = 2; break;
                                case devnull: continue; // impossible
                                case file:
                                    ld.fd = open(ld.path, O_WRONLY | O_APPEND | O_CREAT,
                                            log_file_mask);
                                    if(unlikely(ld.fd == -1)) {
                                        if(unlikely(!ld.error_reported)) {
                                            std::cerr << "cannot open/create log file \""
                                                    << ld.path << '"' << std::endl;
                                            ld.error_reported = true;
                                        }
                                        ld.scheduled_for_writing.clear();
                                        continue;
                                    } else {
                                        ld.number_of_writes_since_last_reopen = 0;
                                        ld.last_reopen = now_us();
                                        ld.error_reported = false;
                                    }
                                    break;
                            }
                        }
                        // fd != -1
                        // writev on files will not be partial
                        int rc;
                        do {
                            rc = writev(ld.fd, ld.scheduled_for_writing.data(),
                                    ld.scheduled_for_writing.size());
                        } while(unlikely(rc == -1 && errno == EINTR));
                        if(unlikely(rc == -1 && !ld.error_reported && ld.dest == file)) {
                            std::cerr << "cannot write to log file \""
                                << ld.path << '"' << std::endl;
                            ld.error_reported = true;
                        }
                        ++ld.number_of_writes_since_last_reopen;
                    }
                    ld.scheduled_for_writing.clear();
                }
            }

            inline bool needs_reopen(const level_data &ld) const {
                size_t now = now_us();
                return ld.number_of_writes_since_last_reopen >
                    max_number_of_writes_since_last_reopen ||
                    now > ld.last_reopen + max_open_time_us;
            }

            static std::string to_short_string(level l) {
                switch(l) {
                    case logger_level_critical: return "CRIT";
                    case logger_level_error: return "ERRR";
                    case logger_level_warning: return "WARN";
                    case logger_level_info: return "INFO";
                    case logger_level_profile: return "PROF";
                    case logger_level_debug: return "DEBG";
                    case logger_level_debug_hard: return "DBHR";
                    case logger_level_debug_mare: return "DBMR";
                    default: return "UNKN";
                }
            }

#ifndef NTEST // logger_level
        public:

            static bool test_create_destroy() {
                { logger_level l; }
                return true;
            }

            static bool test_set_level() {
                logger_level l;
                l.set(logger_level_warning, disabled);
                l.set(logger_level_error, enabled, with_ansi_colors, no_location, file, "/tmp/log");
                l.set(logger_level_critical, enabled, no_ansi_colors, with_location, stderr);
                assert(l.levels[logger_level_warning].enabled == false);
                assert(l.levels[logger_level_error].enabled == true);
                assert(l.levels[logger_level_error].ansi_colors == true);
                assert(l.levels[logger_level_error].location == false);
                assert(l.levels[logger_level_error].dest == file);
                assert(std::string(l.levels[logger_level_error].path) == "/tmp/log");
                assert(l.levels[logger_level_critical].enabled == true);
                assert(l.levels[logger_level_critical].ansi_colors == false);
                assert(l.levels[logger_level_error].location == true);
                assert(l.levels[logger_level_error].dest == stderr);
                return true;
            }

#endif
    };

    extern logger_level the_logger_level;


    class logger {
        private:

            enum ansi_color : uint16_t {
                date_color = 238,
                level_color = 1,
                neutral_color = 0,
                critical_level_color = 124,
                error_level_color = 124,
                warning_level_color = 184,
                info_level_color = 112,
                profile_level_color = 135,
                debug_level_color = 130,
                debug_hard_level_color = 131,
                debug_mare_level_color = 133
            };

            many_to_many_circular_queue<std::ostringstream> circular_queue;
            const size_t default_logger_queue_size = 4096;
            std::thread writer;

        public:

            logger() {
                circular_queue.init(default_logger_queue_size);
                start_writing_thread();
            }

            ~logger() {
                stop_writing_thread();
            }

            size_t print_log_prefix(std::ostringstream &out, logger_level::level level,
                    const char *file, const char *line) {
                size_t ansi_size = 0;
                out << level;
                logger_level::level_data ld = the_logger_level.level_details(level);
                if(unlikely(ld.ansi_colors)) ansi_size += ansi(out, date_color);
                current_time_to_stream(out);
                out << ' ';
                if(unlikely(ld.location)) {
                    // if(unlikely(ld.ansi_colors)) ansi_size += ansi(out, location_color);
                    out << file << ':' << line;
                    out << ' ';
                }
                if(unlikely(ld.ansi_colors)) ansi_size += ansi(out, level_color, level);
                out << ' ' << the_logger_level.to_short_string(level) << ' ';
                return out.str().size() - ansi_size - 1;
            }

            void align_multiline_log_item(std::ostringstream &out, size_t prefix_size) {
                if(out.str().empty() || !prefix_size) return; // no needs to edit
                std::deque<size_t> newlines;
                const std::string &original = out.str();
                size_t pos = 0;
                while((pos = original.find('\n', pos)) != std::string::npos)
                    newlines.push_back(pos);
                if(unlikely(original.back() == '\n')) newlines.pop_back(); // remove trailing \n
                if(newlines.empty()) return; // no needs to edit
                std::string edited(prefix_size * newlines.size() + original.size(), ' ');
                pos = 0;
                size_t dest_pos = 0;
                do {
                    size_t to_copy = (newlines.empty() ? original.size() : newlines.front() + 1)
                        - pos;
                    original.copy(const_cast<char*>(edited.data()) + dest_pos, to_copy, pos);
                    pos += to_copy;
                    dest_pos += to_copy;
                    dest_pos += prefix_size;
                    if(!newlines.empty()) newlines.pop_front();
                } while(pos < original.size());
                out.str(edited);
            }

            void print_log_suffix(std::ostringstream &out, logger_level::level level) {
                logger_level::level_data ld = the_logger_level.level_details(level);
                if(unlikely(ld.ansi_colors)) ansi(out, neutral_color);
            }

            inline bool push(std::ostringstream *pout) {
                return circular_queue.push(pout);
            }

        private:

            void start_writing_thread() {
                writer = std::thread(&logger::writer_thread, this);
            }

            void stop_writing_thread() {
                auto finish_flag = new std::ostringstream();
                finish_flag->setstate(std::ios::eofbit);
                while(!push(finish_flag)) std::this_thread::yield();
                writer.join();
            }

            void writer_thread() {
                lower_this_thread_priority();
                circular_vector<std::ostringstream*> swapped;
                std::vector<std::ostringstream*> delayed_garbage_collect;
                bool keep_running = true;
                while(keep_running) {
                    circular_queue.pop_all(swapped);
                    for(auto pout : swapped) {
                        if(unlikely(!keep_running)) { // delete everything after finish_flag
                            delete pout;
                        } else if(unlikely(!pout)) { // overflow, skipped some entries
                            // TODO
                        } else if(unlikely(pout->eof())) { // finish_flag
                            keep_running = false;
                            delete pout;
                        } else {
                            const std::string &report = pout->str();
                            if(unlikely(report.empty())) abort();
                            logger_level::level lvl = (logger_level::level) report[0];
                            logger_level::level_data &ld = the_logger_level.level_details(lvl);
                            ld.scheduled_for_writing.emplace_back(
                                    iovec{const_cast<char*>(report.data()) + 1, report.size() - 1});
                            delayed_garbage_collect.push_back(pout);
                        }
                    }
                    the_logger_level.write_reports();
                    for(auto pout : delayed_garbage_collect) delete pout;
                    delayed_garbage_collect.clear();
                }
            }

            size_t ansi(std::ostringstream &out, ansi_color type,
                    logger_level::level lvl = logger_level::logger_level_critical) {
                switch(type) {
                    case date_color: out << "\033[38;5;" << (uint16_t) date_color << 'm'; return 11;
                    case neutral_color: out << "\033[m"; return 3;
                    case level_color:
                        switch(lvl) {
                            case logger_level::logger_level_critical:
                                out << "\033[38;5;" << (uint16_t) critical_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_error:
                                out << "\033[38;5;" << (uint16_t) error_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_warning:
                                out << "\033[38;5;" << (uint16_t) warning_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_info:
                                out << "\033[38;5;" << (uint16_t) info_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_profile:
                                out << "\033[38;5;" << (uint16_t) profile_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_debug:
                                out << "\033[38;5;" << (uint16_t) debug_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_debug_hard:
                                out << "\033[38;5;" << (uint16_t) debug_hard_level_color << 'm';
                                return 11;
                            case logger_level::logger_level_debug_mare:
                                out << "\033[38;5;" << (uint16_t) debug_mare_level_color << 'm';
                                return 11;
                            default: return 0; // impossible
                        };
                    default:
                        return 0; // impossible
                }
            }
    };

    extern logger the_logger;


    enum multiline_switch {
        no_multiline = false,
        multiline = true
    };


#define to_log(level, streaming_content, multiline) \
    if(unlikely(the_logger_level.is_enabled(level))) { \
        std::unique_ptr<std::ostringstream> out(new std::ostringstream); \
        [[gnu::unused]] size_t prefix_size = \
            the_logger.print_log_prefix(*out, level, __FILE__, __LINE__); \
        *out << streaming_content; \
        if(multiline) the_logger.align_multiline_log_item(*out, prefix_size); \
        the_logger.print_log_suffix(*out, level); \
        std::ostringstream *pout = out.release(); \
        if(!the_logger.push(pout)) delete pout; \
    }


#define log_critical(streaming_content) \
    { to_log(logger_level::logger_level_critical, streaming_content, no_multiline); abort(); }
#define log_critical_multiline(streaming_content) \
    { to_log(logger_level::logger_level_critical, streaming_content, multiline); abort(); }

#define log_error(streaming_content) \
    to_log(logger_level::logger_level_error, streaming_content, no_multiline)
#define log_error_multiline(streaming_content) \
    to_log(logger_level::logger_level_error, streaming_content, multiline)

#define log_warning(streaming_content) \
    to_log(logger_level::logger_level_warning, streaming_content, no_multiline)
#define log_warning_multiline(streaming_content) \
    to_log(logger_level::logger_level_warning, streaming_content, multiline)

#define log_info(streaming_content) \
    to_log(logger_level::logger_level_info, streaming_content, no_multiline)
#define log_info_multiline(streaming_content) \
    to_log(logger_level::logger_level_info, streaming_content, multiline)

#define log_profile(streaming_content) \
    to_log(logger_level::logger_level_profile, streaming_content, no_multiline)
#define log_profile_multiline(streaming_content) \
    to_log(logger_level::logger_level_profile, streaming_content, multiline)

#define log_debug(streaming_content) \
    to_log(logger_level::logger_level_debug, streaming_content, no_multiline)
#define log_debug_multiline(streaming_content) \
    to_log(logger_level::logger_level_debug, streaming_content, multiline)


#ifdef NDEBUG

#define log_debug_hard(streaming_content)
#define log_debug_hard_multiline(streaming_content)

#define log_debug_mare(streaming_content)
#define log_debug_mare_multiline(streaming_content)

#else

#define log_debug_hard(streaming_content) \
    to_log(logger_level::logger_level_debug_hard, streaming_content, no_multiline)
#define log_debug_hard_multiline(streaming_content) \
    to_log(logger_level::logger_level_debug_hard, streaming_content, multiline)

#define log_debug_mare(streaming_content) \
    to_log(logger_level::logger_level_debug_mare, streaming_content, no_multiline)
#define log_debug_mare_multiline(streaming_content) \
    to_log(logger_level::logger_level_debug_mare, streaming_content, multiline)

#endif

}
