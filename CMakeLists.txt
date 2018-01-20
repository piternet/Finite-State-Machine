cmake_minimum_required(VERSION 2.8.9)
project(finite-state-machine)

set(CMAKE_C_FLAGS "-std=c11" CACHE STRING "" FORCE)

add_executable(validator helper.c validator.c)
add_executable(tester helper.c tester.c)
add_executable(run helper.c run.c)

target_link_libraries(validator rt)
target_link_libraries(tester rt)