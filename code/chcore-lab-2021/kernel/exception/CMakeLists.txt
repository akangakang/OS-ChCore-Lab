cmake_minimum_required(VERSION 3.14)

file(GLOB OBJECT_SOURCE "*.c" "*.S")
add_library(${PROJECT_NAME}-exception OBJECT ${OBJECT_SOURCE})
target_include_directories(
	${PROJECT_NAME}-exception
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)