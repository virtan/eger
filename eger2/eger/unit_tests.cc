#include <iostream>
#include <assert.h>
#include "unit_tests_headers.agcc"

int main() {
    std::cout << "Running unit tests..." << std::endl;
#include "unit_tests_body.agcc"
    std::cout << "Unit tests passed successfully" << std::endl;
    return 0;
}
