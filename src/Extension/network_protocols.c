//
// Created by kenny on 18/06/18.
//
#include "network_protocols.h"
#include "stdlib.h"

int sendPacket (struct dataPacket *packet, int socket){
  char *bytes = (char *) packet;
  int bytesSent = 0;

  while (bytesSent != PACKET_SIZE) {
    int sent = send(socket, bytes+bytesSent, PACKET_SIZE-bytesSent, 0);
    if (sent == -1)
      return -1;

    bytesSent += sent;
  }
  return bytesSent;
}

int recievePacket (struct dataPacket **packet, int socket){
  /** A fixed amount of bytes is sent per packet, no matter what
      message is being sent. This is so we know much much data is expected

   */

  char *bytes = (char *) malloc(PACKET_SIZE);
  int bytesReceived = 0;

  while (bytesReceived != PACKET_SIZE){
    int recieved = recv(socket, bytes+bytesReceived, PACKET_SIZE-bytesReceived, 0);
    if (recieved == -1)
      return -1;

    bytesReceived += recieved;
  }

  if (*packet)
    free(*packet);

  *packet = (struct dataPacket *) bytes;

  return 0;
}

void serverEndConnection (int socket){
  struct dataPacket packet;
  packet.type = STOC_CONNECTION_ENDED;
  packet.argc = 0;
  sendPacket(&packet, socket);
}

void serverTooManyPlayers (int socket){
  struct dataPacket packet;
  packet.type = STOC_TOO_MANY_PLAYERS;
  packet.argc = 0;
  sendPacket(&packet, socket);
}
void clientEndConnection (int socket){
  struct dataPacket packet;
  packet.type = CTOS_END_CONNECTION;
  packet.argc = 0;
  sendPacket(&packet, socket);
}

void sendListOfPlayers(int socket, struct clientThread *clients){
  struct dataPacket packet;
  packet.type = STOC_LIST_OF_PLAYERS;
  packet.argc = 0;

  for (struct clientThread *client = clients->prev; client != NULL; client = client->prev){
    if (strlen(client->username) == 0 || !client->validPlayer)
      continue;

    strcpy(packet.args[packet.argc], client->username);
    packet.argc++;
  }

  for (struct clientThread *client = clients->next; client != NULL; client = client->next){
    if (strlen(client->username) == 0 || !client->validPlayer)
      continue;

    strcpy(packet.args[packet.argc], client->username);
    packet.argc++;
  }

  sendPacket(&packet, socket);

}

void sendNoArgsPacket (int socket, uint8_t message){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = 0;
  sendPacket(&packet, socket);
}

void sendOneArgIntPacket (int socket, uint8_t message, int arg){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = 1;
  * ((int *) (&packet.args[0][0])) = arg;

  sendPacket(&packet, socket);
}

void sendTwoArgIntPacket(int socket, uint8_t message, int arg1, int arg2){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = 2;
  * ((int *) (&packet.args[0][0])) = arg1;
  * ((int *) (&packet.args[1][0])) = arg2;

  sendPacket(&packet, socket);
}


void sendOneArgStrPacket (int socket, uint8_t message, char *str){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = 1;
  strcpy(packet.args[0],str);

  sendPacket(&packet, socket);
}

void sendIntAndStrPacket (int socket, uint8_t message, int argInt, char *argStr){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = 2;
  * ((int *) (&packet.args[0][0])) = argInt;
  strcpy(packet.args[1], argStr);
  sendPacket(&packet, socket);
}

void sendNormalMove(int socket, uint8_t message, int gameId, struct Move move){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = NORMAL_MOVE_ARGC;
  * ((int *) (&packet.args[0][0])) = gameId;
  * ((int *) (&packet.args[1][0])) = move.startRow;
  * ((int *) (&packet.args[2][0])) = move.startCol;
  * ((int *) (&packet.args[3][0])) = move.endRow;
  * ((int *) (&packet.args[4][0])) = move.endCol;
  * ((int *) (&packet.args[5][0])) = (int) move.isEnPassant;
  * ((int *) (&packet.args[6][0])) = move.promotionPiece;

  sendPacket(&packet, socket);
}

void sendCastlingMove(int socket, uint8_t message, int gameId, struct Move move){
  struct dataPacket packet;
  packet.type = message;
  packet.argc = CASTLING_MOVE_ARGC;
  * ((int *) (&packet.args[0][0])) = gameId;
  * ((int *) (&packet.args[0][0])) = gameId;
  * ((int *) (&packet.args[1][0])) = move.startRow;
  * ((int *) (&packet.args[2][0])) = move.startCol;
  * ((int *) (&packet.args[3][0])) = move.endRow;
  * ((int *) (&packet.args[4][0])) = move.endCol;
  * ((int *) (&packet.args[5][0])) = move.startRow2;
  * ((int *) (&packet.args[6][0])) = move.startCol2;
  * ((int *) (&packet.args[7][0])) = move.endRow2;
  * ((int *) (&packet.args[8][0])) = move.endCol2;

  sendPacket(&packet, socket);
}