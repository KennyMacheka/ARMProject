CC= gcc
FLAGS= -std=c99
PROC= Emulator/processor_data_handling.h
FETCH= Emulator/fetch_decode_execute.h
BIT= bit_operations_utilities.h

emulate: out/emulate.o out/processor_data_handling.o out/fetch_decode_execute.o out/bit_operations_utilities.o
	$(CC) $(FLAGS) out/bit_operations_utilities.o out/emulate.o out/processor_data_handling.o out/fetch_decode_execute.o -o emulate

out/processor_data_handling.o : Emulator/processor_data_handling.c $(PROC)
	$(CC) $(FLAGS) -c Emulator/processor_data_handling.c -o out/processor_data_handling.o

out/bit_operations_utilities.o : bit_operations_utilities.c bit_operations_utilities.h
	$(CC) $(FLAGS) -c bit_operations_utilities.c -o out/bit_operations_utilities.o

out/fetch_decode_execute.o : Emulator/fetch_decode_execute.c $(FETCH) $(PROC) $(BIT) Emulator/result_flags.h
	$(CC) $(FLAGS) -c Emulator/fetch_decode_execute.c -o out/fetch_decode_execute.o

out/emulate.o: Emulator/emulate.c $(PROC) $(FETCH) $(BIT) 
	$(CC) $(FLAGS) -c Emulator/emulate.c -o out/emulate.o

out/:
	mkdir out

clean:
	rm -rf out emulate
