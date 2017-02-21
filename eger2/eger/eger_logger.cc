#include <eger_logger.h>

namespace eger {

    bool logger_level::autodetected_ansi_colors = autodetect_ansi();

    logger_level the_logger_level;
    
    logger the_logger;

}
