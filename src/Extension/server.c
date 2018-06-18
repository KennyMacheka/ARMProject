//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
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
  struct addrinfo *server = NULL;
  struct sockaddr_storage clientAddress;
  socklen_t addressSize;

  char ipStr[INET6_ADDRSTRLEN];
  char host[50];

  gethostname(host,50);

  memset(&hints, 0, sizeof(hints));
  //IPv4 or IPv6
  hints.ai_family = AF_UNSPEC;
  //TCP- continuous connection
  hints.ai_socktype = SOCK_STREAM;
  //Will fill in server's IP
  hints.ai_flags = AI_PASSIVE;
  char hostname[200];
  gethostname(hostname,200);
  printf("Host: %s\n", hostname);
  if (getaddrinfo(hostname, PORT_STR, &hints, &serverInfo) != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  void *addr;
  char *ipver;

  for (server = serverInfo; server != NULL; server = server->ai_next){
    mainSocket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

    if (mainSocket == -1){
      printf("Error: server socket.\n");
      continue;
    }

    int bindVal = bind(mainSocket, server->ai_addr, server->ai_addrlen);
    if (bindVal == -1){
      close(mainSocket);
      printf("Error: server bind.\n");
      continue;
    }

    if (server->ai_family == AF_INET) { // IPv4
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)server->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    }
    else { // IPv6
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)server->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }
    inet_ntop(server->ai_family, addr, ipStr, sizeof (ipStr));
    printf(" %s: %s\n", ipver, ipStr);

    break;
  }

  if (server == NULL){
     printf("Failed to establish server socket and bind.\n");
     return EXIT_FAILURE;
  }


  listen(mainSocket, BACKLOG);
  printf("Listening...\n");
  addressSize = sizeof(clientAddress);
  clientSocket = accept(mainSocket, (struct sockaddr *)&clientAddress, &addressSize);
  char *msg = "Hello, there";
  int len, bytesSent;

  len = strlen(msg);
  bytesSent = send(clientSocket, msg, len, 0);
  printf("Bytes sent: %d\n", bytesSent);
  shutdown(clientSocket, 2);
  close(mainSocket);
  freeaddrinfo(serverInfo);


}