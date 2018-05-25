cmake_minimum_required(VERSION 3.10)
project(arm11_35)

set(CMAKE_C_FLAGS "-std=c99")


add_executable(Emulator src/emulate.c)

add_executable(Assembler src/assemble.c)