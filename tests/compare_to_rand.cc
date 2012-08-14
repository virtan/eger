#include <iostream>
#include <eger/logger.h>

using namespace std;

size_t calc(int argc) {
    size_t result = 0;
    size_t i;

    cout << (argc > 1 ? "test with log_debug" : "test with rand") << "\n";
    time_t start_time = time(NULL);
    for(i = 0;;++i) {
        if(argc > 1) log_debug("to debug: " << result);
        else result += rand();
        if(i % 1000000 == 0) {
            time_t current_time = time(NULL);
            cout << "\r" << (current_time - start_time) << "     ";
            cout.flush();
            if(current_time - start_time > 20)
                break;
        }
    }

    cout << "\r" << (i/1000000) << " millions iterations\n";

    return result;

}

int main(int, char **) {
    eger::instance eger_logger;
    eger_logger.start_writer();

    return calc(1) + calc(2);
}
