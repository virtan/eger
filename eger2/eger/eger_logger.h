
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
                    number_of_writes(0),
                    last_write(0) // TODO: fast time
                {}

                bool enabled;
                bool ansi_colors;
                bool location;
                destination dest;
                char path[256];
                int fd;
                size_t number_of_writes;
                size_t last_write;
            };

        private:
            std::array<level_data, logger_levels> levels;
            bool autodetected_ansi_colors;

        public:

            // by default we're printing critical, error and warning only
            // we print them to stderr, ansi colors are enabled
            logger_level() : autodetected_ansi_colors(autodetect_ansi()) {
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
                    if(ld.dest == file) {
                        ld.path[0] = 0;
                        path.copy(ld.path[1], std::min(path.size() - 2, sizeof(ld.path - 2)), 1);
                        ld.path[sizeof(ld.path) - 1] = 0;
                        ld.path[0] = path[0];
                    }
                    ld.dest = dest;
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
                sregex_token_iterator it(settings.begin(), settings.end(), re);
                sregex_token_iterator reg_end;
                for(; it != reg_end; ++it) {
                    auto &rer = it->str();
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
    };

    extern logger_level the_logger_level;

    class logger {
        private:

            many_to_many_circular_queue<std::ostringstream> circular_queue;
            const size_t default_logger_queue_size = 4096;

        public:

            logger() {
                circular_queue.init(default_logger_queue_size);
                start_logging_thread();
            }

            ~logger() {
                stop_logging_thread();
            }

            size_t print_log_prefix(std::ostringstream &out, logger_level::level level,
                    const char *file, const char *line) {
                size_t ansi_size = 0;
                level_data ld = the_logger_level.level_details(level);
                if(unlikely(ld.ansi_colors)) ansi_size += ansi(out, date_color);
                current_time_to_stream(out);
                out << ' ';
                if(unlikely(ld.location)) {
                    // if(unlikely(ld.ansi_colors)) ansi_size += ansi(out, location_color);
                    out << file << ':' << line;
                    out << ' ';
                }
                if(unlikely(ld.ansi_colors)) ansi_size += ansi(out, level_color, level);
                out << ' ' << the_logger_level.to_string(level) << ' ';
                return out.str().size() - ansi_size;
            }

            void align_multiline_log_item(std::ostringstream &out, size_t prefix_size) {
                if(out.str().empty() || !prefix_size) return; // no needs to edit
                std::vector<size_t> newlines;
                std::string &original = out.str();
                size_t pos = 0;
                while((pos = original.find('\n', pos)) != std::string::npos)
                    newlines.push_back(pos);
                if(unlikely(original.back() == '\n')) newlines.pop_back(); // remove trailing \n
                if(newlines.empty()) return; // no needs to edit
                std::string edited(prefix_size * newlines.size() + original.size(), ' ');
                pos = 0;
                size_t dest_pos = 0;
                do {
                    // TODO
                    original.copy(&edited[dest_pos], newlines.front() - pos, pos);

                } while(newlines.empty());

                if(!newlines.empty()) {
                    edited.resize(prefix_size * newlines.size() + edited.size());

                }
                out.str(edited);
            }

            void print_log_suffix(std::ostringstream &out, logger_level::level level) {
                level_data ld = the_logger_level.level_details(level);
                if(unlikely(ld.ansi_colors)) ansi(out, neutral_color);
            }

            inline bool push(std::ostringstream *pout) {
                return circular_queue.push(pout);
            }

        private:

            void start_logging_thread();
            void stop_logging_thread();
            size_t ansi(std::ostringstream &out, ansi_color type,
                    logger_level::level = logger_level::logger_level_critical);
    };

    extern logger the_logger;
    

    enum multiline_switch {
        no_multiline = false,
        multiline = true
    };


#define to_log(level, streaming_content, multiline) \
    if(unlikely(the_logger_level.is_enabled(level))) { \
        std::unique_ptr<std::ostringstream> out = new std::ostringstream; \
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
#define log_info(streaming_content) \
    to_log(logger_level::logger_level_info, streaming_content, multiline)

#define log_profile(streaming_content) \
    to_log(logger_level::logger_level_profile, streaming_content, no_multiline)
#define log_profile(streaming_content) \
    to_log(logger_level::logger_level_profile, streaming_content, multiline)

#define log_debug(streaming_content) \
    to_log(logger_level::logger_level_debug, streaming_content, no_multiline)
#define log_debug(streaming_content) \
    to_log(logger_level::logger_level_debug, streaming_content, multiline)


#ifdef NDEBUG

#define log_debug_hard(streaming_content)
#define log_debug_hard_multiline(streaming_content)

#define log_debug_mare(streaming_content)
#define log_debug_mare_multiline(streaming_content)

#else

#define log_debug_hard(streaming_content) \
    to_log(logger_level::logger_level_debug_hard, streaming_content, no_multiline)
#define log_debug_hard(streaming_content) \
    to_log(logger_level::logger_level_debug_hard, streaming_content, multiline)

#define log_debug_mare(streaming_content) \
    to_log(logger_level::logger_level_debug_mare, streaming_content, no_multiline)
#define log_debug_mare(streaming_content) \
    to_log(logger_level::logger_level_debug_mare, streaming_content, multiline)

#endif

}
