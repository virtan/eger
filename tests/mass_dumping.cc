#include <iostream>
#include <eger/logger.h>

int main() {
    eger::instance eger_logger;
    eger_logger.resize(eger::log_level_size);
    eger_logger[(size_t) eger::level_critical] = "stderr";
    eger_logger[(size_t) eger::level_error] = "error.log";
    eger_logger[(size_t) eger::level_warning] = "error.log";

    eger_logger.start_sync_writer();

    for(size_t i = 0; i < 10*1000*1000; usleep(1)) {
        log_warning("this is a warning");
        log_error("this is a error");
        log_critical("this is critical");
        log_debug("and this is harasho");
    }
}
