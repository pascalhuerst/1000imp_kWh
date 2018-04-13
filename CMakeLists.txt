cmake_minimum_required(VERSION 2.8)

project(1000imp_kWh)

FILE(GLOB projectsources *.c)

add_compile_options(-Wall)

add_executable(${PROJECT_NAME} ${projectsources})
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
