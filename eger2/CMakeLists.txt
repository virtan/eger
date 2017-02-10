cmake_minimum_required (VERSION 3.2)
project (eger)
set (eger_VERSION_MAJOR 2)
set (eger_VERSION_MINOR 1)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
#sed -nE "/ifndef +NTEST/,/endif/p;" many_to_many_circular_queue.h | sed -nE "s/^.*static +bool +(test_.*)\(.*$/\\1/p"
