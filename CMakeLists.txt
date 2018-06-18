

cmake_minimum_required(VERSION 3.10)
project(arm11_35)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_C_FLAGS " -std=c99 -lSDL2 -pthread -lSDL2main -Wall -pedantic")


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

add_executable(Client
        src/Extension/Chess_Engine/chess_engine.c
        src/Extension/Chess_Engine/chess_engine.h
        src/Extension/Client/client.c
        src/Extension/network_protocols.h
        src/Extension/network_protocols.c)

add_executable(Server
               src/Extension/server.c
               src/Extension/network_protocols.h src/Extension/network_protocols.c)

target_link_libraries(Assembler m)
target_link_libraries(Client SDL2 SDL2_ttf SDL2_image)
#target_link_libraries(Extension ${SDL2_LIBRARIES} SDL2_tff SDL2_image)

