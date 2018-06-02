cmake_minimum_required(VERSION 3.7)
project(arm11_35)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        doc/Checkpoint.tex
        doc/Makefile
        doc/Report.tex
        programs/gpio.s
        src/assembler/assemble.c
        src/Emulator/emulate.c
        src/Makefile src/assembler/file_processing.c src/assembler/binary_conversion.c src/assembler/first_pass.c)

add_executable(arm11_35 ${SOURCE_FILES})