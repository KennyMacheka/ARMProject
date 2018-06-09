CC= gcc
FLAGS= -std=c99 -lm
PROC= Emulator/processor_data_handling.h
FETCH= Emulator/fetch_decode_execute.h
BIT= Utilities/bit_operations_utilities.h
FILE_H= assembler/file_handling.h
SYMBOL_T= assembler/symbol_table_tokens.h
CONVERT_I= assembler/convert_instructions.h

$(shell mkdir -p out)
all: assemble emulate

emulate: out/emulate.o out/processor_data_handling.o out/fetch_decode_execute.o out/bit_operations_utilities.o
	$(CC) $(FLAGS) out/bit_operations_utilities.o out/emulate.o out/processor_data_handling.o out/fetch_decode_execute.o -o emulate

out/processor_data_handling.o : Emulator/processor_data_handling.c $(PROC)
	$(CC) $(FLAGS) -c Emulator/processor_data_handling.c -o out/processor_data_handling.o

out/bit_operations_utilities.o : Utilities/bit_operations_utilities.c $(BIT)
	$(CC) $(FLAGS) -c Utilities/bit_operations_utilities.c -o out/bit_operations_utilities.o

out/fetch_decode_execute.o : Emulator/fetch_decode_execute.c $(FETCH) $(PROC) $(BIT) Emulator/result_flags.h
	$(CC) $(FLAGS) -c Emulator/fetch_decode_execute.c -o out/fetch_decode_execute.o

out/emulate.o: Emulator/emulate.c $(PROC) $(FETCH) $(BIT) 
	$(CC) $(FLAGS) -c Emulator/emulate.c -o out/emulate.o

assemble: out/assemble.o out/convert_instructions.o out/file_handling.o out/symbol_table_tokens.o out/bit_operations_utilities.o out/processor_data_handling.o
	$(CC) $(FLAGS) out/assemble.o out/convert_instructions.o out/file_handling.o out/symbol_table_tokens.o out/bit_operations_utilities.o out/processor_data_handling.o -o assemble

out/convert_instructions.o: assembler/convert_instructions.c $(BIT) $(PROC) $(CONVERT_I) $(SYMBOL_T)
	$(CC) $(FLAGS) -c assembler/convert_instructions.c -o out/convert_instructions.o

out/symbol_table_tokens.o: assembler/symbol_table_tokens.c $(SYMBOL_T)
	$(CC) $(FLAGS) -c assembler/symbol_table_tokens.c -o out/symbol_table_tokens.o

out/file_handling.o: assembler/file_handling.c  $(FILE_H)
	$(CC) $(FLAGS) -c assembler/file_handling.c -o out/file_handling.o

out/assemble.o: assembler/assemble.c $(FILE_H) $(CONVERT_I)
	$(CC) $(FLAGS) -c assembler/assemble.c -o out/assemble.o



clean:
	rm -rf out emulate
	rm -rf out assemble
