cmake_minimum_required(VERSION 3.11)

file(GLOB user_lib_source_files "*.c" "*.S" )
add_library(chcore-user-lib STATIC ${user_lib_source_files})
