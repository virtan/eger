
// Typical logger call:
// log_warning("a warning number " << 5)

namespace eger {

    enum logger_level : uint8_t {
        logger_level_critical = 0,
        logger_level_error,
        logger_level_info,
        logger_level_warning,
        logger_level_profile,
        logger_level_debug,
        logger_level_debug_hard,
        logger_level_debug_mare
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
