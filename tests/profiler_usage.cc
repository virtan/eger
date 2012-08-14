#include <iostream>
#include <eger/profiler.h>

size_t actual_work();
void actual_work_iteration(size_t &res, size_t i);

int main() {
    eger::instance eger_logger;
    eger_logger[(size_t) eger::level_profile] = "stderr";
    eger_logger.start_writer();

    size_t res = actual_work();
    std::cout << "Result is " << res << std::endl;
    profiler_dump(multiplication_cumulative);
    profiler_dump(division_cumulative);

    return 0;
}

size_t actual_work() {
    size_t res = 1;
    for(size_t i = 1; i < 20480000; ++i)
        actual_work_iteration(res, i);
    return res;
}

void actual_work_iteration(size_t &res, size_t i) {
    profiler_start(multiplication_cumulative);
    res = res ? res * i : res + i;
    profiler_stop(multiplication_cumulative);
    profiler_start(division_cumulative);
    while(res && !(res % 7)) res /= 7;
    profiler_stop(division_cumulative);
}
