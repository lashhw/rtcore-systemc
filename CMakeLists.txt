cmake_minimum_required(VERSION 3.23)
project(rtcore_systemc)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(src/third_party/bvh)
link_libraries(systemc)
link_libraries(bvh)

add_executable(tb_arbiter testbench/tb_arbiter.cpp)
add_executable(tb_blocking testbench/tb_blocking.cpp)
add_executable(tb_mem testbench/tb_mem.cpp)
add_executable(tb_sync_fifo testbench/tb_sync_fifo.cpp)
add_executable(tb_top testbench/tb_top.cpp)
