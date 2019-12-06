cmake_minimum_required(VERSION 3.13)
project(AdvancedProgramming)
set(CMAKE_CXX_STANDARD 14)


add_executable(FirstTest FirstTest.c)
add_executable(server server.c)
add_executable(sockets sockets.c)
add_executable(fatal_error fatal_error.c)
add_executable(mongoose-example mongoose-example.c)
add_executable(mongoose mongoose.c)
add_executable(post-mongoose post-mongoose.c)
add_executable(new-example new-example.c)