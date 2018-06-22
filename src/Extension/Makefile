CC= gcc
FLAGS= -std=c99 -pthread

$(shell mkdir -p out)

server.exe: out/server.o out/network_protocols.o out/chess_engine.o
		$(CC) $(FLAGS) out/server.o  out/network_protocols.o out/chess_engine.o -o server.exe

out/server.o: server.c network_protocols.h Chess_Engine/chess_engine.h
	$(CC) $(FLAGS) -c server.c -o out/server.o

out/network_protocols.o: network_protocols.c network_protocols.h
	$(CC) $(FLAGS) -c network_protocols.c -o out/network_protocols.o

out/chess_engine.o: Chess_Engine/chess_engine.c Chess_Engine/chess_engine.h
	$(CC) $(FLAGS) -c Chess_Engine/chess_engine.c -o out/chess_engine.o

clean:
	rm -rf out server.exe

