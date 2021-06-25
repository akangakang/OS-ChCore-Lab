cmake_minimum_required(VERSION 3.14)

project(test_aarch64_page_table C)
set(SOURCE_PATH ../../../kernel/mm)
set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/test_aarch64_page_table.dir)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -g")

set(SOURCES
    test_aarch64_page_table.c
)

add_executable(test_aarch64_page_table ${SOURCES})
include_directories(
    ../../../kernel/mm
    ../../../kernel/
    ../../include
    ../../../
)

add_custom_target(
    lcov
    COMMAND lcov -d ${CMAKE_CURRENT_SOURCE_DIR} -z
    COMMAND lcov -d ${CMAKE_CURRENT_SOURCE_DIR} -b . --initial -c -o lcov.info
    COMMAND CTEST_OUTPUT_ON_FAILURE=1 ${CMAKE_MAKE_PROGRAM} test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_custom_command(
    TARGET lcov
    COMMAND lcov -d ${CMAKE_CURRENT_SOURCE_DIR} -c -o lcov.info
    COMMAND genhtml -o report --prefix=`pwd` lcov.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS test_aarch64_page_table
)

enable_testing()
add_test(test_aarch64_page_table ${CMAKE_CURRENT_BINARY_DIR}/test_aarch64_page_table)
