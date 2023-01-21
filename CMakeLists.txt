cmake_minimum_required(VERSION 3.23)
project(rtcore_systemc)

set(CMAKE_CXX_STANDARD 17)

add_executable(rtcore_systemc testbench/tb_top.cpp)
target_link_libraries(rtcore_systemc systemc)

add_executable(tb_sync_fifo testbench/tb_sync_fifo.cpp)
target_link_libraries(tb_sync_fifo systemc)