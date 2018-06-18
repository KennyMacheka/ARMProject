//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
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

static pthread_mutex_t lock;
static int numClients = 0;
struct clientThread{
  char username[MAX_USERNAME_SIZE+1];
  int socket;
  pthread_t *thread;
  struct clientThread *next;
  struct clientThread *prev;
};

void *clientServerInteraction (void *clientSocket);
struct clientThread *initialiseThreadStruct();

struct clientThread *initialiseThreadStruct(){
  struct clientThread *client = (struct clientThread *) malloc(sizeof(struct clientThread));
  client->thread = NULL;
  client->prev = NULL;
  client->next = NULL;

  return client;
}

void freeThreadStruct (struct clientThread *client){
  if (!client)
    return;

  if (client->prev)
    client->prev->next = client->next;

  free(client->thread);
  free(client);
}


void *clientServerInteraction (void *clientSocket){
  //Add time out, so if nothing happens after certain time, disconnet client
  printf("Interacting with client.\n");
  struct clientThread *client = (struct clientThread *) clientSocket;
  struct DataPacket *packet = (struct DataPacket *) (malloc(sizeof(struct DataPacket)));
  packet->type = STOC_CONNECTION_ESTABLISHED;
  packet->argc = 0;

  //First thing is to inform client that connection has been established
  bool validConnection = true;
  if (sendPacket (packet, client->socket) == -1)
    validConnection = false;

  //Expect to get username from client
  //As both client and server are following a protocol, the client knows it has to do this
  //So if it doesn't sent the right datum, then connection will be closed
  if (recievePacket(&packet, client->socket) != 0){
    validConnection = false;
  }

  if (packet->type != CTOS_SEND_USERNAME)
    validConnection = false;

  strcpy(client->username, packet->args[0]);

  //Now we wait for client to send requests
  //If we get any unexpected messages from the client, we ask them to resend
  //If this is a match, then we do this a few more times and then they are disqualified
  //Otherwise, if not a match, then no harm done
  while(validConnection){

  }

  pthread_mutex_lock(&lock);
  serverEndConnection(client->socket);
  shutdown(client->socket, 2);
  freeThreadStruct(client);
  numClients--;
  pthread_mutex_unlock(&lock);

  if (packet)
    free(packet);
  return NULL;
}

int main(){
  int status;
  int mainSocket;
  struct clientThread *clients = initialiseThreadStruct();

  struct addrinfo hints;
  struct addrinfo *serverInfo = NULL;
  struct addrinfo *server = NULL;
  struct sockaddr_storage clientAddress;
  socklen_t addressSize;

  memset(&hints, 0, sizeof(hints));
  //IPv4 or IPv6
  hints.ai_family = AF_UNSPEC;
  //TCP- continuous connection
  hints.ai_socktype = SOCK_STREAM;
  //Will fill in server's IP
  hints.ai_flags = AI_PASSIVE;

  if(pthread_mutex_init(&lock, NULL) != 0){
    fprintf(stderr,"Unable to intialise locking.");
    return EXIT_FAILURE;
  }

  if (getaddrinfo(HOST_NAME, PORT_STR, &hints, &serverInfo) != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

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
    break;
  }

  if (server == NULL){
     printf("Failed to establish server socket and bind.\n");
     return EXIT_FAILURE;
  }

  while(1) {
    listen(mainSocket, BACKLOG);
    printf("Listening...\n");
    addressSize = sizeof(clientAddress);
    clients->socket = accept(mainSocket, (struct sockaddr *) &clientAddress, &addressSize);
    clients->thread = (pthread_t *) malloc(sizeof(pthread_t));
    pthread_mutex_lock(&lock);
    if (pthread_create(clients->thread, NULL, clientServerInteraction, clients) != 0){
      serverEndConnection(clients->socket);
      shutdown(clients->socket, 2);
      freeThreadStruct(clients);
      continue;
    }
    printf("Connected.\n");

    if (clients->prev)
      clients->prev->next = clients;

    clients->next = initialiseThreadStruct();
    clients = clients->next;
    pthread_mutex_unlock(&lock);
  }

  /**If i do end up breaking, remember to join threads, or terminate them.*/
  close(mainSocket);
  freeaddrinfo(serverInfo);


}