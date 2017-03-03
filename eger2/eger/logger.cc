#include <stdio.h>
#include <eger/logger.h>

namespace eger {

    bool logger_level::autodetected_ansi_colors_stdout = autodetect_ansi(fileno(::stdout));
    bool logger_level::autodetected_ansi_colors_stderr = autodetect_ansi(fileno(::stderr));

    logger_level the_logger_level;
    logger the_logger(the_logger_level);

}
