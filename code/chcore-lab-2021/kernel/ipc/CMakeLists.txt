cmake_minimum_required(VERSION 3.14)


file(GLOB OBJECT_SOURCE "*.c")
add_library(${PROJECT_NAME}-ipc OBJECT ${OBJECT_SOURCE})
target_include_directories(
	${PROJECT_NAME}-ipc
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)