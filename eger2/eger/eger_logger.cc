#include <eger_logger.h>

namespace eger {

    logger::logger() {
        init();
    }
    
    logger::~logger() {
        deinit();
    }

    void logger::init() {
        active_logger_buffer_size = logger_buffer_size = default_logger_buffer_size;
        logger_buffer_commit_position = logger_buffer_reader_position =
            logger_buffer_writer_position = 0;
        logger_max_level = default_logger_max_level;
        logger_buffer.resize(active_logger_buffer_size);
        register_global(logger_buffer_size);
        start_logging_thread();
    }

    void logger::deinit() {
    }

    template<class wrapper_lambda>
    void logger::send(logger_level level, const wrapper_lambda &logdata_wrapper_lambda) {
        if((uint8_t) level > logger_max_level) return;
        
        logdata_lambda()
        
    }

    logger logger_instance;
    size_t logger_buffer_size;
    uint8_t logger_max_level;

}
