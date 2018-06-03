cmake_minimum_required(VERSION 3.10)
project(arm11_35)

set(CMAKE_C_FLAGS "-std=c99 -Wall")

set(SOURCE_FILES
        doc/Checkpoint.tex
        doc/Makefile
        doc/Report.tex
        programs/gpio.s
        src/assembler/assemble.c
        src/assembler/binary_conversion.c
        src/assembler/file_processing.c
        src/assembler/first_pass.c
        src/assembler/symbol_table.c
        src/Emulator/emulate.c
        src/Emulator/fetch_decode_execute.c
        src/Emulator/fetch_decode_execute.h
        src/Emulator/processor_data_handling.c
        src/Emulator/processor_data_handling.h
        src/Emulator/result_flags.h
        src/Utilities/bit_operations_utilities.c
        src/Utilities/bit_operations_utilities.h
        src/Makefile src/assembler/file_processing.c src/assembler/binary_conversion.c src/assembler/first_pass.c)

add_executable(arm11_35 ${SOURCE_FILES})
