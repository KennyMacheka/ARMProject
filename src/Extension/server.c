//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "network_protocols.h"

/*
    Here, user means client
    The server will listen out for players to connect.
    The player must send their player name. If name is not taken,
    connection is established.
    The user can then request:
      List of available players
      A request to play a specific player
      A move being sent to a game

 */

int main(){
  int status;
  int mainSocket, clientSocket;
  struct addrinfo hints;
  struct addrinfo *serverInfo = NULL;
  struct addrInfo *server = NULL;
  struct sockaddr_storage clientAddress;
  socklen_t addressSize;

  char ipStr[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  //IPv4 or IPv6
  hints.ai_family = AF_UNSPEC;
  //TCP- continuous connection
  hints.ai_socktype = SOCK_STREAM;
  //Will fill in server's IP
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT_STR, &hints, &serverInfo) != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  void *addr;
  char *ipver;

  /*
  for (struct addrinfo *p = serverInfo; p != NULL; p = p->ai_next) {
    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }

    if (inet_ntop(p->ai_family, addr, ipStr, sizeof(ipStr)))
      printf(" %s: %s\n", ipver, ipStr);
    else {
      fprintf(stderr, "inet_ntop conversion errror: %s\n", strerror(errno));
    }
  }*/

  mainSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

  if (mainSocket == -1){
    fprintf(stderr, "Failed to establish main socket. Error msg: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  if(bind(mainSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
    fprintf(stderr, "Failed to bind to port. Error msg: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  listen(mainSocket, BACKLOG);

  addressSize = sizeof(clientAddress);
  clientSocket = accept(mainSocket, (struct sockaddr *)&clientAddress, &addressSize);
  char *msg = "Hello, there";
  int len, bytesSent;

  len = strlen(msg);
  bytesSent = send(clientSocket, "Hello", len, 0);

  shutdown(clientSocket, 2);
  close(mainSocket);
  freeaddrinfo(serverInfo);


}