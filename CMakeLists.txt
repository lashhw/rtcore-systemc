cmake_minimum_required(VERSION 3.23)
project(rtcore-systemc)

set(CMAKE_CXX_STANDARD 17)

add_executable(rtcore-systemc main.cpp custom_structs/vec3.hpp custom_structs/triangle.hpp modules/rtcore/ist.hpp modules/rtcore/rtcore.hpp custom_structs/bvh.hpp custom_structs/bounding_box.hpp modules/rtcore/trv.hpp modules/rtcore/rd.hpp modules/testbench.hpp custom_structs/ray_state.hpp modules/rtcore/post.hpp modules/rtcore/fifos/rd_post_fifo.hpp modules/rtcore/list.hpp modules/rtcore/fifos/list_fifo.hpp modules/raygen.hpp modules/shader.hpp)
target_link_libraries(rtcore-systemc systemc)

add_executable(gen-references gen_references/main.cpp)
add_subdirectory(gen_references/bvh)
target_link_libraries(gen-references PUBLIC bvh)