cmake_minimum_required(VERSION 3.10)
project(arm11_35)

set(CMAKE_C_FLAGS "-std=c99 -Wall")


add_executable(Emulator
               src/emulate.c
        src/processor_data_handling.c
        src/processor_data_handling.h src/bit_operations_utilities.c src/bit_operations_utilities.h src/fetch_decode_execute.h  src/fetch_decode_execute.c src/fetch_decode_execute.h)

add_executable(Assembler src/assemble.c src/processor_data_handling.c src/processor_data_handling.h)