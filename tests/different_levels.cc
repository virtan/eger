#include <iostream>
#include <eger/logger.h>

int main() {
    eger::instance eger_logger;
    eger_logger[(size_t) eger::level_critical] = "stderr";
    eger_logger[(size_t) eger::level_error] = "error.log";
    eger_logger[(size_t) eger::level_warning] = "error.log";

    eger_logger.start_writer();

    log_warning("this is a warning");
    log_error("this is a error");
    log_critical("this is critical " <<  __PRETTY_FUNCTION__);
    log_debug("and this is harasho");
}
