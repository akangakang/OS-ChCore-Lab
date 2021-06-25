cmake_minimum_required(VERSION 3.14)

file(GLOB OBJECT_SOURCE "*.c" "*.S")
add_library(${PROJECT_NAME}-mm OBJECT ${OBJECT_SOURCE})
target_include_directories(
	${PROJECT_NAME}-mm
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

#for testing
#add_library(${PROJECT_NAME}-mm OBJECT mm.c vmregion.c buddy.c kmalloc.c slab.c
#../../tests/mm/mm-slab/test_slab.c ../../tests/mm/mm-buddy/test_buddy.c
#../../tests/mm/mm-pagetable/test_pagetable.c)
