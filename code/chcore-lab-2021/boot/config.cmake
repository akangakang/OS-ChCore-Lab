cmake_minimum_required(VERSION 3.14)

set(INIT_PATH       "${BOOTLOADER_PATH}")

file(
    GLOB
        tmpfiles
        "${INIT_PATH}/*.c"
        "${INIT_PATH}/*.S"
)

list(APPEND files ${tmpfiles})
