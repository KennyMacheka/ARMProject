//
// Created by kenny on 13/06/18.
//

#ifndef ARM11_35_NETWORK_PROTOCOLS_H
#define ARM11_35_NETWORK_PROTOCOLS_H
#include <stdint.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "Chess_Engine/chess_engine.h"


#define PORT_STR "23556"
#define BACKLOG 20
#define HOST_NAME "kenny-Aspire-ES1-521"
#define MAX_PLAYERS 20
#define MAX_USERNAME_SIZE 15
#define PACKET_SIZE 705
#define MAX_STRINGS 70
#define SERVER_CLIENT_TIMEOUT 10

#define NORMAL_MOVE_ARGC 7 //gameId, start_row, start_col, end_row, end_col, is_EnPassant, pawnPromotionPiece
#define CASTLING_MOVE_ARGC 9 //Same as normal move but an extra start and end positions, and no en passant or pawn promotion check (implitictly no)
#define CLIENT_PING_INTERVAL 3 //Client sends a "I'm still online" packet every 3 seconds
#define CLIENT_REQUEST_PLAYERS_INTERVAL 5

#define STOC_CONNECTION_ESTABLISHED 1 //No arguments
#define CTOS_SEND_USERNAME 2
#define STOC_USERNAME_TAKEN 3
#define CTOS_GET_PLAYERS 4
#define CTOS_CHALLENGE_PLAYER 5 //player_username
#define STOC_GAME_STARTED 6 //gameId, colour
#define CTOS_MOVE 7 //gameId, start_row, start_col, end_row, end_col If promotion, also new piece num
#define STOC_OPPONENT_MOVE 8 //gameId, start row, start col, end rol, end, col
#define CTOS_END_CONNECTION 10
#define STOC_CONNECTION_ENDED 11
//This'll end connection, client will be aware
#define STOC_TOO_MANY_PLAYERS 12
#define STOC_OPPONENT_LEFT 13
//A ping sent to the server every few seconds to inform it client is still online
#define CTOS_STILL_ONLINE 14
#define CTOS_OFFER_DRAW 15 //gameId
#define CTOS_RESIGN 16 //gameId
#define CTOS_CLAIM_DRAW_50_MOVE 17 //gameId
#define CTOS_REJECT_DRAW_OFFER 18 //gameId
#define STOC_LIST_OF_PLAYERS 19
#define STOC_CHALLENGE_REQUEST 20 //gameId
#define CTOS_REJECT_MATCH_REQUEST 21 //gameId
#define CTOS_ACCEPT_REQUEST 22 //gameId
#define STOC_CANNOT_CHALLENGE_PLAYER 23
#define STOC_DRAW_OFFERED 24 //gameId
#define CTOS_ACCEPT_DRAW_OFFER 25 //gameId
#define STOC_DRAW_OFFER_ACCEPTED 26
#define STOC_OPPONENT_RESIGNED 27
#define STOC_OPPONENT_CLAIMED_50_MOVE 28
#define STOC_DRAW_OFFER_REJECTED 29
#define STOC_USERNAME_VALID 30


//STOC = server to client
//CTOS = client to server
//When a game is over, the client and server do not need to communicate this
struct dataPacket {
  uint8_t type;
  uint32_t argc;
  char args[MAX_STRINGS][MAX_USERNAME_SIZE+1];

};

struct clientThread{
  //Add some kind of timing structure, here, look it up.
  //if whatever you find uses processor ticks, account for this
  //At the beginning of clientServerInteraction, start the timer,
  //Follow the instructions at the bottom of the loop
  uintmax_t lastPacket;
  char username[MAX_USERNAME_SIZE+1];
  int socket;
  pthread_t *thread;
  struct clientThread *next;
  struct clientThread *prev;
  bool validPlayer;
};


int sendPacket (struct dataPacket *packet, int socket);
int recievePacket (struct dataPacket **packet, int socket);
void sendNoArgsPacket (int socket, uint8_t message);
void sendOneArgIntPacket (int socket, uint8_t message, int arg);
void sendListOfPlayers(int socket, struct clientThread *clients);

void sendNormalMove(int socket, uint8_t message, int gameId, struct Move move);

void sendCastlingMove(int socket, uint8_t message, int gameId, struct Move move);

#endif //ARM11_35_NETWORK_PROTOCOLS_H
