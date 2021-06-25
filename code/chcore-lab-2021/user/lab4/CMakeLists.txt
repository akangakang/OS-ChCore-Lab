cmake_minimum_required(VERSION 3.11)

set(TEST_LAB4_BINS
    "yield_single"
    "yield_spin"
    "yield_multi"
    "yield_aff"
    "yield_multi_aff"
    "spawn_basic" "spawn_info" "spawn_child"
    "ipc_data" "ipc_data_server"
    "ipc_reg" "ipc_reg_server"
     "ipc_mem" "ipc_mem_server"
)

foreach(bin ${TEST_LAB4_BINS})
  file(GLOB ${bin}_source_files "${bin}.c")
  add_executable(${bin}.bin ${${bin}_source_files})
  target_link_libraries(${bin}.bin chcore-user-lib)
  set_property(
          TARGET ${bin}.bin
          APPEND_STRING
          PROPERTY
          LINK_FLAGS
          "-e START"
  )
endforeach(bin)
