cmake_minimum_required(VERSION 3.13)
project(AdvancedProgramming)
set(CMAKE_CXX_STANDARD 14)


add_executable(mongoose mongoose.c)
add_executable(post-mongoose post-mongoose.c mongoose.c)
add_executable(multithreaded multithreaded.c)
add_executable(game_of_life_omp game_of_life_omp.c)
add_executable(string_functions string_functions.c)
add_executable(pgm_image pgm_image.c)