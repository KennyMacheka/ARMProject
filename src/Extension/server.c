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
#include "Chess_Engine/chess_engine.h"

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

enum TURN{
  PLAYER1,
  PLAYER2
};

struct match{
  enum TURN turn;
  struct clientThread *player1;
  struct clientThread *player2;
  enum COLOUR player1_colour;
  enum COLOUR player2_colour;
  bool matchStarted;
  struct Game *game;
};

struct matchesStruct{
  int gameId;
  bool initialised;
  struct match game;
  struct matchesStruct *next;
  struct matchesStruct *prev;
};

struct matchesStruct *newestMatch = NULL;

void *clientServerInteraction (void *clientSocket);
struct clientThread *initialiseThreadStruct();
void freeMatchStruct(struct matchesStruct *match);
struct matchesStruct *setupMatchStruct();
struct clientThread *getPlayer (struct clientThread *clients, char *username);
struct matchesStruct *getMatch (int gameId);
void sendMatchMessage (struct clientThread *client, struct matchesStruct *match,
                       uint8_t message);

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

  if (newestMatch == match)
    newestMatch = match->prev;

  if (match->prev)
    match->prev->next = match->next;

  if (match->next)
    match->next->prev = match->prev;

  if (match->initialised){
    free(match->game.game);
  }

  free(match);
}

struct matchesStruct *getMatch (int gameId){
  for (struct matchesStruct *match = newestMatch; match != NULL; match = match->prev){
    if (match->gameId == gameId)
      return match;
  }

  return NULL;
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
  client->validPlayer = false;
  return client;
}

void freeThreadStruct (struct clientThread *client){
  if (!client)
    return;

  if (client->prev)
    client->prev->next = client->next;

  if (client->next)
    client->next->prev = client->prev;

  free(client->thread);
  free(client);
}

void sendMatchMessage (struct clientThread *client, struct matchesStruct *match,
                       uint8_t message){

  if (match->game.player1 != client)
    sendOneArgIntPacket(match->game.player1->socket, message, match->gameId);
  else if (match->game.player2 != client)
    sendOneArgIntPacket(match->game.player2->socket, message, match->gameId);

}

void *clientServerInteraction (void *clientSocket){ //Consider adding a separate lock for game handling
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

  pthread_mutex_lock(&lock);

  for (struct clientThread *c = client->next; c != NULL; c = c->next){
    if(strcmp(c->username, packet->args[0]) == 0 && c->validPlayer) {
      sendNoArgsPacket(client->socket, STOC_USERNAME_TAKEN);
      validConnection = false;
    }
  }

  for (struct clientThread *c = client->prev; c != NULL; c = c->prev){
    if(strcmp(c->username, packet->args[0]) == 0 && c->validPlayer) {
      sendNoArgsPacket(client->socket, STOC_USERNAME_TAKEN);
      validConnection = false;
    }
  }

  if (validConnection) {
    sendNoArgsPacket(client->socket, STOC_USERNAME_VALID);
    strcpy(client->username, packet->args[0]);
    client->validPlayer = true;
  }

  pthread_mutex_unlock(&lock);

  //Now we wait for client to send requests
  //If we get any unexpected messages from the client, we ask them to resend
  //If this is a match, then we do this a few more times and then they are disqualified
  //Otherwise, if not a match, then no harm done
  client->lastPacket = (uintmax_t )time(NULL);
  int id;
  struct matchesStruct *match;
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
          struct clientThread *opponent = getPlayer(client, packet->args[0]);
          if (!opponent){
            sendNoArgsPacket(client->socket, STOC_CANNOT_CHALLENGE_PLAYER);
          }else{

            struct matchesStruct *newMatch = setupMatchStruct();
            newMatch->prev = newestMatch;
            if (newestMatch)
              newestMatch->next = newMatch;

            newestMatch = newMatch;
            newestMatch->game.player1 = client;
            newestMatch->game.player2 = opponent;
            newestMatch->gameId = gameId;
            newestMatch->game.matchStarted = false;
            sendIntAndStrPacket(client->socket, STOC_FORWARDING_CHALLENGE_REQUEST, gameId, opponent->username);
            sendIntAndStrPacket(opponent->socket, STOC_CHALLENGE_REQUEST, gameId, client->username);
            gameId++;
          }
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_ACCEPT_REQUEST: //Client has accepted match request
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          if (match){ //Setup game, allocate colours, wait for player to make a move
            int first = rand() % 2;
            match->game.game = setupGame();
            if (first == 0) { //Player 1 will be white
              match->game.player1_colour = WHITE;
              match->game.player2_colour = BLACK;
              match->game.turn = PLAYER1;
              //Player 1 is this current socket
              sendTwoArgIntPacket(match->game.player1->socket, STOC_GAME_STARTED, id, WHITE);
              sendTwoArgIntPacket(match->game.player2->socket, STOC_GAME_STARTED, id, BLACK);
            }else{
              match->game.player2_colour = WHITE;
              match->game.player1_colour = BLACK;
              match->game.turn = PLAYER2;
              sendTwoArgIntPacket(match->game.player1->socket, STOC_GAME_STARTED, id, BLACK);
              sendTwoArgIntPacket(match->game.player2->socket, STOC_GAME_STARTED, id, WHITE);
            }
          }
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_MOVE: //Client had sent a move for a particular game, need to forward this to opponent
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          struct Move move;
          int startRow = * ((int *) &packet->args[1][0]);
          int startCol = * ((int *) &packet->args[2][0]);
          int endRow = * ((int *) &packet->args[3][0]);
          int endCol = * ((int *) &packet->args[4][0]);

          if(packet->argc == NORMAL_MOVE_ARGC){
            bool isEnpassant = (bool) *((int *) &packet->args[5][0]);
            enum PIECES promotionPiece = * ((int *) &packet->args[6][0]);

            if (promotionPiece == BLANK)
              move = setupMoveStruct(match->game.game, startRow, startCol, endRow, endCol, isEnpassant);

            else
              move = setupMovesStructPromotion(match->game.game, startRow, startCol, endRow, endCol, promotionPiece);

          }else if (packet->argc == CASTLING_MOVE_ARGC){
            int startRow2 = * ((int *) &packet->args[5][0]);
            int startCol2 = * ((int *) &packet->args[6][0]);
            int endRow2 = * ((int *) &packet->args[7][0]);
            int endCol2 = * ((int *) &packet->args[8][0]);
            move = setupMoveStructCastling(match->game.game, startRow, startCol, endRow, endCol,
                                           startRow2, startCol2, endRow2, endCol2);
          }

          requestMove(match->game.game, &move);

          if (match->game.turn == PLAYER1){
            if (packet->argc == NORMAL_MOVE_ARGC)
              sendNormalMove(match->game.player2->socket, STOC_OPPONENT_MOVE, match->gameId, move);

            else
              sendCastlingMove(match->game.player2->socket, STOC_OPPONENT_MOVE, match->gameId, move);

            match->game.turn = PLAYER2;
          }else{
            if (packet->argc == NORMAL_MOVE_ARGC)
              sendNormalMove(match->game.player1->socket, STOC_OPPONENT_MOVE, match->gameId, move);

            else
              sendCastlingMove(match->game.player1->socket, STOC_OPPONENT_MOVE, match->gameId, move);

            match->game.turn = PLAYER1;
          }

          if(match->game.game->matchState != NOT_OVER)
            freeMatchStruct(match);

          pthread_mutex_unlock(&lock);
          break;

        case CTOS_OFFER_DRAW:
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);

          if (match)
            sendMatchMessage(client, match, STOC_DRAW_OFFERED);
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_RESIGN:
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          if (match) {
            sendMatchMessage(client, match, STOC_OPPONENT_RESIGNED);
            freeMatchStruct(match);
          }
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_CLAIM_DRAW_50_MOVE:
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          if(match){
            sendMatchMessage(client, match, STOC_OPPONENT_CLAIMED_50_MOVE);
            if (match->game.game->fiftyMoveCount >= 50){
              sendMatchMessage(client, match, STOC_OPPONENT_CLAIMED_50_MOVE);
              freeMatchStruct(match);
            }
          }
          pthread_mutex_unlock(&lock);
          break;

        case CTOS_REJECT_DRAW_OFFER:
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          if (match)
            sendMatchMessage(client, match, STOC_DRAW_OFFER_REJECTED);

          pthread_mutex_unlock(&lock);
          break;

        case CTOS_ACCEPT_DRAW_OFFER:
          pthread_mutex_lock(&lock);
          id = * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          if (match){
            sendMatchMessage(client, match, STOC_DRAW_OFFER_ACCEPTED);
            freeMatchStruct(match);
          }

          pthread_mutex_unlock(&lock);
          break;

        case CTOS_REJECT_MATCH_REQUEST: //Other play doesn't want to play
          pthread_mutex_lock(&lock);
          id =  * ((int *) &packet->args[0][0]);
          match = getMatch(id);
          if (match){
            sendMatchMessage(client, match, STOC_CANNOT_CHALLENGE_PLAYER);
            freeMatchStruct(match);
          }

          pthread_mutex_unlock(&lock);
          break;

        case CTOS_END_CONNECTION:
          validConnection = false;
          client->validPlayer = false;
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
  for (match = newestMatch; match != NULL;){
    struct matchesStruct *prev = match->prev;
    if (match->game.player1 == client || match->game.player2 == client) {
      if (match->game.player1 == client) {
        if (match->game.player2->validPlayer)
          sendOneArgIntPacket(match->game.player2->socket, STOC_OPPONENT_LEFT, match->gameId);
      }

      else {
        if (match->game.player1->validPlayer)
          sendOneArgIntPacket(match->game.player1->socket, STOC_OPPONENT_LEFT, match->gameId);
      }

      freeMatchStruct(match);
    }

    match = prev;
  }

  freeThreadStruct(client);
  numClients--;
  pthread_mutex_unlock(&lock);
  if (packet)
    free(packet);
  printf("Disconnected client.\n");
  return NULL;
}

int main(){
  srand(time(NULL));
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
        clients->next->prev = clients;
        clients = clients->next;
      }
    }

    pthread_mutex_unlock(&lock);
  }

  /**If i do end up breaking, remember to join threads, or terminate them.*/
  for (struct matchesStruct *match = newestMatch; match != NULL; match = match->prev)
    freeMatchStruct(newestMatch);

  close(mainSocket);
  freeaddrinfo(serverInfo);


}