//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
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
static int gameId = 0;

struct match{
  enum TURN{
    PLAYER,
    PLAYER2
  }turn;
  int gameId;
  struct clientThread *player1;
  struct clientThread *player2;
  enum COLOUR player1_colour;
  enum COLOUR player2_colour;
  bool matchStarted;
  struct Game *game;
};

struct matchesStruct{
  bool initialised;
  struct match game;
  struct machesStruct *next;
  struct matchesStruct *prev;
};

struct matchesStruct *newestMatch = NULL;
struct matchesStruct *head = NULL;

void *clientServerInteraction (void *clientSocket);
struct clientThread *initialiseThreadStruct();
void freeMatchStruct(struct matchesStruct *match);
struct matchesStruct *setupMatchStruct();
struct clientThread *getPlayer (struct clientThread *clients, char *username);


struct matchesStruct *setupMatchStruct(){
  struct matchesStruct *match = (struct matchesStruct *) malloc(sizeof(struct matchesStruct));
  match->initialised = false;
  match->next = NULL;
  match->prev = NULL;
  return match;
}

void freeMatchStruct(struct matchesStruct *match){
  if (!match)
    return;

  if (match->prev)
    match->prev->next = match->next;

  if (match->initialised){
    free(match->game.game);
  }

  free(match);
}

struct clientThread *getPlayer (struct clientThread *clients, char *username){
  if(!clients)
    return NULL;

  for (struct clientThread *client = clients->prev; client != NULL; client = client->prev){
    if (strcmp(client->username,username) == 0)
      return client;
  }

  for (struct clientThread *client = clients->next; client != NULL; client = client->next){
    if (strcmp(client->username,username) == 0)
      return client;
  }

  return NULL;
}

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
  struct dataPacket *packet = (struct dataPacket *) (malloc(sizeof(struct dataPacket)));
  packet->type = STOC_CONNECTION_ESTABLISHED;
  packet->argc = 0;
  uintmax_t  timeElapsed;
  //Start clock here

  //First thing is to inform client that connection has been established
  bool validConnection = true;
  if (sendPacket (packet, client->socket) == -1)
    validConnection = false;

  //Expect to get username from client
  //As both client and server are following a protocol, the client knows it has to do this
  //So if it doesn't sent the right datum, then connection will be closed
  if (recievePacket(&packet, client->socket) != 0)
    validConnection = false;

  //The occasional "Still online" packets are sent after sending a username
  if (packet->type != CTOS_SEND_USERNAME)
    validConnection = false;

  strcpy(client->username, packet->args[0]);

  //Now we wait for client to send requests
  //If we get any unexpected messages from the client, we ask them to resend
  //If this is a match, then we do this a few more times and then they are disqualified
  //Otherwise, if not a match, then no harm done
  client->lastPacket = (uintmax_t )time(NULL);
  while(validConnection){

    if (recievePacket(&packet, client->socket) == 0){
      client->lastPacket = (uintmax_t) time(NULL); //packet has been sent,

      switch(packet->type){

        case CTOS_STILL_ONLINE:
          break;

        case CTOS_GET_PLAYERS:
          pthread_mutex_lock(&lock);
          sendListOfPlayers(client->socket, client);
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_CHALLENGE_PLAYER:
          //Check player is still online
          pthread_mutex_lock(&lock);
          struct clientThread *opponent = getPlayer(client, packet->args[1]);
          if (!opponent){
            sendNoArgsPacket(client->socket, STOC_CANNOT_CHALLENGE_PLAYER);
          }else{
            newestMatch = setupMatchStruct();
            newestMatch->game.player1 = client;
            newestMatch->game.player2 = opponent;
            newestMatch->game.gameId = gameId;
            newestMatch->game.matchStarted = false;
            serverForwardMatchRequest(opponent->socket, gameId);
            gameId++;
          }
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_ACCEPT_REQUEST:
          pthread_mutex_lock(&lock);
          int id = * ((int *) &packet->args[0][0]);
          //Now I need get game struct and start game
          break;

        case CTOS_MOVE:
          break;

        case CTOS_OFFER_DRAW:
          break;

        case CTOS_RESIGN:
          break;

        case CTOS_CLAIM_DRAW_50_MOVE:
          break;

        case CTOS_REJECT_DRAW_OFFER:
          break;

        case CTOS_END_CONNECTION:
          validConnection = false;
          break;

        default:
          break;
      }
    }


    timeElapsed = (uintmax_t ) time(NULL);
    if (timeElapsed - client->lastPacket >= SERVER_CLIENT_TIMEOUT)
      validConnection = false;
  }

  pthread_mutex_lock(&lock);
  sendNoArgsPacket(client->socket, STOC_CONNECTION_ENDED);
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
  head = setupMatchStruct();
  newestMatch = head->next;
  newestMatch->prev = head;

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
    clients->username[0] = '\0';
    pthread_mutex_lock(&lock);
    if (numClients == MAX_PLAYERS){
      sendNoArgsPacket(clients->socket, STOC_TOO_MANY_PLAYERS);
      shutdown(clients->socket, 2);
      freeThreadStruct(clients);
    }

    else {
      if (pthread_create(clients->thread, NULL, clientServerInteraction, clients) != 0) {
        sendNoArgsPacket(clients->socket, STOC_CONNECTION_ENDED);
        shutdown(clients->socket, 2);
        freeThreadStruct(clients);
      } else {
        numClients++;
        printf("Connected.\n");

        if (clients->prev)
          clients->prev->next = clients;

        clients->next = initialiseThreadStruct();
        clients = clients->next;
      }
    }

    pthread_mutex_unlock(&lock);
  }

  /**If i do end up breaking, remember to join threads, or terminate them.*/
  freeMatchStruct(head);
  close(mainSocket);
  freeaddrinfo(serverInfo);


}