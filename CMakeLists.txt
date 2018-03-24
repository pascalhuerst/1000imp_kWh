cmake_minimum_required(VERSION 2.8)

project(1000imp_kWh)
find_package(Threads) # pthread

FILE(GLOB projectsources *.cpp)

add_compile_options(-Wall)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>)

add_executable(${PROJECT_NAME} ${projectsources})
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

