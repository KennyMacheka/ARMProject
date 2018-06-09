

cmake_minimum_required(VERSION 3.10)
project(arm11_35)

set(CMAKE_C_FLAGS " -std=c99 -Wall -pedantic")


add_executable(Emulator
        src/Emulator/emulate.c
        src/Emulator/processor_data_handling.c
        src/Emulator/processor_data_handling.h
        src/Utilities/bit_operations_utilities.c
        src/Utilities/bit_operations_utilities.h
        src/Emulator/fetch_decode_execute.h
        src/Emulator/fetch_decode_execute.c
        src/Emulator/fetch_decode_execute.h)

add_executable(Assembler
               src/assembler/assemble.c
        src/assembler/convert_instructions.c
               src/assembler/convert_instructions.h
               src/assembler/file_handling.c
               src/assembler/file_handling.h
               src/assembler/symbol_table_tokens.c
               src/assembler/symbol_table_tokens.h
               src/Utilities/bit_operations_utilities.c
               src/Utilities/bit_operations_utilities.h
               src/Emulator/processor_data_handling.c
               src/Emulator/processor_data_handling.h)

target_link_libraries(Assembler m)

