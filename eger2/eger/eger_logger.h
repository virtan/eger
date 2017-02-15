
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

            struct level_data {
                level_data() :
                    enabled(false),
                    ansi_colors(false),
                    dest(stderr),
                    path{0},
                    fd(-1),
                    number_of_writes(0),
                    last_write(0) // TODO: fast time
                {}

                bool enabled;
                bool ansi_colors;
                destination dest;
                char path[256];
                int fd;
                size_t number_of_writes;
                size_t last_write;
            };

        private:
            std::array<level_data, logger_levels> levels;

        public:

            // by default we're printing critical, error and warning only
            // we print them to stderr, ansi colors are enabled
            logger_level() {
                for(level i : {logger_level_critical, logger_level_error, logger_level_warning}) {
                    auto &ld = levels[i];
                    ld.enabled = true;
                    ld.ansi_colors = true;
                }
            }
            
            ~logger_level() {
                for(size_t i = 0; i < logger_levels; ++i)
                    levels[i].enabled = false;
            }

            // sets a loglevel details
            // examples:
            //     set(logger_level_warning, disabled);
            //     set(logger_level_error, enabled, with_ansi_colors, file, "/tmp/log");
            //     set(logger_level_critical, enabled, no_ansi_colors, stderr);
            void set(level l, bool enabled, bool ansi_colors = false,
                    destination dest = stderr, const std::string &path = std::string()) {
                if(enabled) {
                    auto &ld = levels[l];
                    ld.enabled = false;
                    ld.ansi_colors = ansi_colors;
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
            void set(level l, const std::string &settings) {
                static std::regex re("(\"[^\"]*\")|enabled|disabled|"
                        "with_ansi_colors|no_ansi_colors|stderr|stdout|devnull|file");
                bool enabled = false;
                bool ansi_colors = false;
                destination dest = stderr;
                std::string path;
                sregex_token_iterator it(settings.begin(), settings.end(), re);
                sregex_token_iterator reg_end;
                for(; it != reg_end; ++it) {
                    auto &rer = it->str();
                    if(rer == "enabled") enabled = true;
                    else if(rer == "disabled") enabled = false;
                    else if(rer == "with_ansi_colors") ansi_colors = true;
                    else if(rer == "no_ansi_colors") ansi_colors = false;
                    else if(rer == "stderr") dest = stderr;
                    else if(rer == "stdout") dest = stdout;
                    else if(rer == "devnull") dest = devnull;
                    else if(rer == "file") dest = file;
                    else if(rer[0] == '"') {
                        dest = file;
                        path.assign(rer, 1, rer.size() - 2);
                    }
                }
                if(dest == file && path.empty()) dest = devnull;
                set(l, enabled, ansi_colors, dest, path);
            }

            inline bool is_enabled(level l) const {
                return levels[l].enabled;
            }

            level_data &level_details(level l) {
                return levels[l];
            }
    };

    struct logger_stream : public std::ostringstream {
        logger_stream(logger_level _lvl) :
            lvl(_lvl),
            moment(std::chrono::system_clock::now()),
            multiline(false)
        {}
        logger_level lvl;
        std::chrono::time_point moment;
        bool multiline;
    };

    class logger {
        private:
            const size_t default_logger_buffer_size = 4096;
            const uint8_t default_logger_max_level = (size_t) logger_level_warning;
            size_t active_logger_buffer_size;
            size_t logger_buffer_writer_position;
            size_t logger_buffer_commit_position;
            size_t logger_buffer_reader_position;
            std::vector<std::unique_ptr<logger_stream>> logger_buffer;

        public:
            logger();
            ~logger();

            // multiline string should end with '\n'
            template<class wrapper_lambda>
            void send(logger_level level, const wrapper_lambda &logdata_wrapper_lambda);

        private:
            void init();
            void deinit();
            void start_logging_thread();
    };

    extern logger logger_instance;
    extern size_t logger_buffer_size;
    extern uint8_t logger_max_level;
    
}
