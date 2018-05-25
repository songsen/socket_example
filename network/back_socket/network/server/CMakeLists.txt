cmake_minimum_required(VERSION 2.8)
PROJECT(SERVER)
SET(LIB_SRC server.c)
add_executable(server ${LIB_SRC})
