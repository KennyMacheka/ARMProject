//
// Created by kenny on 13/06/18.
//

#ifndef ARM11_35_NETWORK_PROTOCOLS_H
#define ARM11_35_NETWORK_PROTOCOLS_H
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#define PORT_STR "23555"
#define BACKLOG 20
#define HOST_NAME "kenny-Aspire-ES1-521"
#define MAX_PLAYERS 70
#define MAX_USERNAME_SIZE 9
#define PACKET_SIZE 705
#define MAX_STRINGS 70

#define STOC_CONNECTION_ESTABLISHED 1 //No arguments
#define CTOS_SEND_USERNAME 2
#define STOC_USERNAME_INVALID 3
#define CTOS_GET_PLAYERS 4
#define CTOS_CHALLENGE_PLAYER 5
#define STOC_CHALLENGE_ACCEPTED_COLOUR 6
#define CTOS_MOVE 7
#define STOC_OPPONENT_MOVE 8
#define CTOS_END_CONNECTION 10
#define STOC_CONNECTION_ENDED 11
//This'll end connection, client will be aware
#define STOC_TOO_MANY_PLAYERS 12



//STOC = server to client
//CTOS = client to server
//When a game is over, the client and server do not need to communicate this
struct DataPacket {
  uint8_t type;
  uint32_t argc;
  char args[MAX_STRINGS][MAX_USERNAME_SIZE+1];
};

int sendPacket (struct DataPacket *packet, int socket);
int recievePacket (struct DataPacket **packet, int socket);
void serverEndConnection (int socket);
void clientEndConnection (int socket);
void serverTooManyPlayers (int socket);

#endif //ARM11_35_NETWORK_PROTOCOLS_H
