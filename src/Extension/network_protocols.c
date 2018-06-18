//
// Created by kenny on 18/06/18.
//
#include "network_protocols.h"
#include "stdlib.h"

int sendPacket (struct DataPacket *packet, int socket){
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

int recievePacket (struct DataPacket **packet, int socket){
  /** A fixed amount of bytes is sent per packet, no matter what
      message is being sent. This is so we know much much data is expected

      The amount of data to be recieved is calculated by taking an upperbound
      of the largest amount of data that can actually be sent,
      which is the list of players available.

      So max size is:
      1 byte for type of data being sent
      4 bytes for number of char arguments being sent
      700 bytes for maximum number of players and and their username

      so 705 bytes. Take upperbound to be 710 bytes
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

  *packet = (struct DataPacket *) bytes;

  return 0;
}

void serverEndConnection (int socket){
  struct DataPacket packet;
  packet.type = STOC_CONNECTION_ENDED;
  packet.argc = 0;
  sendPacket(&packet, socket);
}

void serverTooManyPlayers (int socket){
  struct DataPacket packet;
  packet.type = STOC_TOO_MANY_PLAYERS;
  packet.argc = 0;
  sendPacket(&packet, socket);
}
void clientEndConnection (int socket){
  struct DataPacket packet;
  packet.type = CTOS_END_CONNECTION;
  packet.argc = 0;
  sendPacket(&packet, socket);
}