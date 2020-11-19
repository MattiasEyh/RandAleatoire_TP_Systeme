cmake_minimum_required(VERSION 3.17)
project(Rand_aleatoire_TP C)

set(CMAKE_C_STANDARD 99)
set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
add_executable(Rand_aleatoire_TP main.c)