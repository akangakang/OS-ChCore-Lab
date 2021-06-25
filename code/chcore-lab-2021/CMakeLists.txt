cmake_minimum_required (VERSION 3.14)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_BUILD_TYPE "Debug")  # "Release" or "Debug"
set(CHCORE_PLAT "raspi3")
set(CHCORE_ARCH "aarch64")

mark_as_advanced(CMAKE_INSTALL_PREFIX)

if(TEST)
    add_definitions("-DTEST=${TEST}")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-DLOG_LEVEL=2")
else ()
    add_definitions("-DLOG_LEVEL=1")
endif ()

set(CROSS_COMPILE "${CHCORE_ARCH}-linux-gnu-")

set(CMAKE_C_COMPILER "${CROSS_COMPILE}gcc")
set(CMAKE_ASM_COMPILER "${CROSS_COMPILE}gcc")
execute_process (
	COMMAND  ${CMAKE_C_COMPILER} -print-file-name=include
	OUTPUT_STRIP_TRAILING_WHITESPACE
	OUTPUT_VARIABLE C_COMPILER_INCLUDE
)


set(CMAKE_C_FLAGS
    "-Wall -fPIC -nostdlib -nostartfiles -ffreestanding \
    -DCHCORE -nostdinc ")

project (chos C ASM)

set(BOOTLOADER_PATH     "boot")
set(KERNEL_PATH         "${CMAKE_CURRENT_SOURCE_DIR}/kernel")
set(USER_PATH           "${CMAKE_CURRENT_SOURCE_DIR}/user")


set(KERNEL_ARCH_PATH        "${KERNEL_PATH}")
set(KERNEL_MM_PATH          "${KERNEL_PATH}/mm")
set(KERNEL_SYSCALL_PATH     "${KERNEL_PATH}/syscall")
set(KERNEL_EXCEPTION_PATH   "${KERNEL_PATH}/exception")
set(KERNEL_PROCESS_PATH     "${KERNEL_PATH}/process")
set(KERNEL_SCHED_PATH       "${KERNEL_PATH}/sched")
set(KERNEL_IPC_PATH         "${KERNEL_PATH}/ipc")
set(KERNEL_TESTS_PATH       "${KERNEL_PATH}/tests")

include_directories(".")
include_directories("${KERNEL_PATH}")

add_subdirectory("${KERNEL_ARCH_PATH}")
add_subdirectory("${KERNEL_MM_PATH}")
add_subdirectory("${KERNEL_PROCESS_PATH}")
add_subdirectory("${KERNEL_SYSCALL_PATH}")
add_subdirectory("${KERNEL_EXCEPTION_PATH}")
add_subdirectory("${KERNEL_SCHED_PATH}")
add_subdirectory("${KERNEL_IPC_PATH}")
add_subdirectory("${KERNEL_TESTS_PATH}")

set(BINARY_KERNEL_IMG_PATH "CMakeFiles/kernel.img.dir")
set(init_object
        "${BINARY_KERNEL_IMG_PATH}/${BOOTLOADER_PATH}/start.S.o
        ${BINARY_KERNEL_IMG_PATH}/${BOOTLOADER_PATH}/mmu.c.o
        ${BINARY_KERNEL_IMG_PATH}/${BOOTLOADER_PATH}/tools.S.o
        ${BINARY_KERNEL_IMG_PATH}/${BOOTLOADER_PATH}/init_c.c.o
        ${BINARY_KERNEL_IMG_PATH}/${BOOTLOADER_PATH}/uart.c.o"
    )

set(link_script "linker.lds")
configure_file("./scripts/linker-aarch64.lds.in" "linker.lds.S")

set(files "")
include("${BOOTLOADER_PATH}/config.cmake")

add_executable(kernel.img ${files} 
    $<TARGET_OBJECTS:${PROJECT_NAME}-arch>
    $<TARGET_OBJECTS:${PROJECT_NAME}-mm>
    $<TARGET_OBJECTS:${PROJECT_NAME}-object>
    $<TARGET_OBJECTS:${PROJECT_NAME}-syscall>
    $<TARGET_OBJECTS:${PROJECT_NAME}-sched>
    $<TARGET_OBJECTS:${PROJECT_NAME}-exception>
    $<TARGET_OBJECTS:${PROJECT_NAME}-ipc>
    $<TARGET_OBJECTS:${PROJECT_NAME}-tests>
    ${USER_PATH}/binary_include.S
)

set_property(
    TARGET kernel.img
    APPEND_STRING
    PROPERTY
        LINK_FLAGS
        "-T ${CMAKE_CURRENT_BINARY_DIR}/${link_script} -e _start"
)

set_property(
    TARGET kernel.img
    APPEND_STRING
    PROPERTY
        DEPENDS
        "${link_script}"
)

add_custom_command(
    TARGET kernel.img
    PRE_LINK
	COMMAND ${CMAKE_C_COMPILER} -E -x c
	-I${CMAKE_SOURCE_DIR}/${BOOTLOADER_PATH}/include ./linker.lds.S | grep -v "^#" > linker.lds
)
