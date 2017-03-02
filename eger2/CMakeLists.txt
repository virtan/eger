cmake_minimum_required(VERSION 3.2)
project(eger)
set(eger_VERSION_MAJOR 2)
set(eger_VERSION_MINOR 1)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/eger)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

find_package(Threads)

set(SOURCES
    eger/useful.cc
    eger/logger.cc
    )

add_executable(unit_tests
    eger/unit_tests.cc
    eger/unit_tests_body.agcc
    eger/unit_tests_headers.agcc
    ${SOURCES}
    )

if(LINUX)
    set(VALGRIND valgrind --leak-check=yes)
else()
    set(VALGRIND)
endif()

add_test(NAME unit_tests COMMAND ${VALGRIND} ./unit_tests)
add_custom_command(
    OUTPUT eger/unit_tests_body.agcc eger/unit_tests_headers.agcc
#sed -nE "/ifndef +NTEST/,/endif/p;" many_to_many_circular_queue.h | sed -nE "s/^.*((static +bool +(test_.*)\(.*)|(NTEST \/\/ +([^ ]*)))$/\\3:\\5/p" | awk 'substr($1,1,1)==":"{prefix=$0; next;} {printf "{bool rc = eger:%s::%s(); assert(rc);}\n",prefix,substr($0,1,length($0)-1);}'
    COMMAND sed -nE '/ifndef +NTEST/,/endif/p;' eger/*.cc eger/*.h | sed -nE 's/^.*\(\(static +bool +\(test_.*\)\\\(.*\)|\(NTEST \\/\\/ +\([^ ]*\)\)\)$$/\\3:\\5/p' | awk 'substr\($$1,1,1\)==":"{prefix=$$0\; next\;} {printf \"{bool rc = eger:%s::%s\(\)\; assert\(rc\)\;}\\n\",prefix,substr\($$0,1,length\($$0\)-1\)\;}' > eger/unit_tests_body.agcc
    COMMAND ls -1 eger/*.h | grep -v indexer | cut -d '/' -f 2- | awk '{print \"\#include \\\"\"$$0\"\\\"\"\;}\' > eger/unit_tests_headers.agcc
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS eger/*.cc eger/*.h
    )

set(CMAKE_CTEST_COMMAND unit_tests)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_custom_target(clean-all COMMAND make clean && rm -rf CMakeCache.txt CMakeFiles Makefile cmake_install.cmake)
