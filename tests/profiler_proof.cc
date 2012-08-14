#include <iostream>
#include <eger/profiler.h>

int main() {
    eger::instance eger_logger;
    eger_logger[(size_t) eger::level_profile] = "stderr";
    eger_logger.start_writer();

    profiler_start(some_point_1);
    int x = 0;
    for(int i = 0; i < 100; ++i)
        x += rand();
    profiler_stop(some_point_1);
    profiler_start(some_point_1);
    profiler_stop(some_point_1);
    profiler_dump(some_point_1);

    profiler_reset(some_point_1);
    profiler_start(some_point_1);
    for(int i = 0; i < 2000000; ++i)
        x += rand();
    profiler_dump(some_point_1);

    profiler_start(some_point_2);
    for(int i = 0; i < 50000000; ++i)
        x += rand();
    profiler_dump(some_point_2);

    return x;
}
