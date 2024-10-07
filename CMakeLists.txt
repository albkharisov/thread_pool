cmake_minimum_required(VERSION 3.28)
project(se_solver)

add_executable(se_solver main.cpp worker_pool.cpp square_solver.cpp)

target_compile_options(se_solver PUBLIC "-Wall" "-pedantic" "-fsanitize=thread")
target_link_options(se_solver PUBLIC "-Wall" "-pedantic" "-fsanitize=thread")
# target_compile_options(se_solver PUBLIC "-g" "-Wall" "-pedantic" "-fsanitize=address,undefined")
# target_link_options(se_solver PUBLIC "-g" "-Wall" "-pedantic" "-fsanitize=address,undefined")

set_property(TARGET se_solver PROPERTY CXX_STANDARD 17)

