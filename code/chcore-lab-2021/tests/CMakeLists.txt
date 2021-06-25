cmake_minimum_required(VERSION 3.14)

project(Tests C)

set(ARCH "aarch64")
set(PLAT "raspi3")

set(KERNEL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../kernel")
set(KERNEL_MM_PATH          "${KERNEL_PATH}/mm")
set(KERNEL_INCLUDE_PATH "${KERNEL_PATH}/common")
include_directories("${KERNEL_INCLUDE_PATH}")


add_library(minunit INTERFACE)
target_include_directories(minunit INTERFACE
    "include")

add_subdirectory(sched)
