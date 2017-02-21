cmake_minimum_required(VERSION 3.2)
project(eger)
set(eger_VERSION_MAJOR 2)
set(eger_VERSION_MINOR 1)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/eger)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(unit_tests eger/unit_tests.cc eger/unit_tests_body.agcc eger/unit_tests_headers.agcc)
add_test(NAME unit_tests COMMAND valgrind --leak-check=yes ./unit_tests)
add_custom_command(
    OUTPUT eger/unit_tests_body.agcc eger/unit_tests_headers.agcc
#sed -nE "/ifndef +NTEST/,/endif/p;" many_to_many_circular_queue.h | sed -nE "s/^.*((static +bool +(test_.*)\(.*)|(NTEST \/\/ +([^ ]*)))$/\\3:\\5/p" | awk 'substr($1,1,1)==":"{prefix=$0; next;} {printf "{bool rc = eger:%s::%s(); assert(rc);}\n",prefix,substr($0,1,length($0)-1);}'
    COMMAND sed -nE '/ifndef +NTEST/,/endif/p;' eger/*.{cc,h} | sed -nE 's/^.*\(\(static +bool +\(test_.*\)\\\(.*\)|\(NTEST \\/\\/ +\([^ ]*\)\)\)$$/\\3:\\5/p' | awk 'substr\($$1,1,1\)==":"{prefix=$$0\; next\;} {printf \"{bool rc = eger:%s::%s\(\)\; assert\(rc\)\;}\\n\",prefix,substr\($$0,1,length\($$0\)-1\)\;}' > eger/unit_tests_body.agcc
    COMMAND ls -1 eger/*.h | grep -v indexer | cut -d '/' -f 2- | awk '{print \"\#include \\\"\"$$0\"\\\"\"\;}\' > eger/unit_tests_headers.agcc
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

set(CMAKE_CTEST_COMMAND unit_tests)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
