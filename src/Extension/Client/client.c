//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "SDL_Libraries.h"
#include "sdl_utilities.h"
#include "../Chess_Engine/chess_engine.h"
#include "../network_protocols.h"
#include "rendering_global_vars.h"
#include "texture.h"

#define USERNAME_PROMPT 0
#define USER_NAME_TAKEN 1
#define CANNOT_CONNECT 2
#define TOTAL_OUTPUT_TEXTS 3
#define MAX_OUTPUT_TEXT_LEN 50


//Working directory is extension
void setupNetwork();
void dismantleNetwork();
void *sendPings (void *dummy);

struct serverDetails{
  int status;
  struct addrinfo hints;
  struct addrinfo *serverInfo;
  struct addrinfo *server;
  struct dataPacket *packet;
  char username[MAX_USERNAME_SIZE+1];
  int numPlayers;
  char playersAvailable[MAX_PLAYERS][MAX_USERNAME_SIZE+1];
  int socket;
}network;

void setupNetwork(){
  network.serverInfo = NULL;
  network.server = NULL;

  memset(&network.hints, 0, sizeof(network.hints));
  //IPv4 or IPv6
  network.hints.ai_family = AF_UNSPEC;
  //TCP- continuous connection
  network.hints.ai_socktype = SOCK_STREAM;
  //Will fill in server's IP
  network.hints.ai_flags = AI_PASSIVE;
  network.socket = -1;
  network.status = getaddrinfo("kenny-Aspire-ES1-521", PORT_STR, &network.hints, &network.serverInfo);

  if (network.status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(network.status));
    dismantleNetwork();
    exit(1);
  }

  printf("Recieved address info.\n");

  for(network.server = network.serverInfo; network.server != NULL; network.server = network.server->ai_next){
    network.socket = socket(network.server->ai_family, network.server->ai_socktype, network.server->ai_protocol);
    if (network.socket == -1) {
      printf("client socket error.\n");
      continue;
    }

    int serverSocket = connect(network.socket, network.server->ai_addr, network.server->ai_addrlen);
    if (serverSocket == -1){
      printf("Client connect error.\n");
      continue;
    }

    break;
  }

  /**Whether the client establishes connection with network or not
     they will be prompted to enter their username.
     If connection was not established, program will work
     behind the scenes to once again establish network
     after the user submits their username.
   */

  if (network.server == NULL){
    printf("Failed to establish socket and connection.\n");
    dismantleNetwork();
  }

  else {

    //Server should tell client where or not network was established
    printf("Attempting to recieve message: \n");
    if (recievePacket(&network.packet, network.socket) != 0) {
      fprintf(stderr, "Received nothing from server.\n");
    }

    printf("Message recieved from server: %d\n", network.packet->type);

  }
}

void *sendPings (void *dummy){
  uintmax_t lastPacket = (uintmax_t) time(NULL);

  while (network.socket != -1){
    uintmax_t current = (uintmax_t) time(NULL);

    if (current - lastPacket >= CLIENT_PING_INTERVAL){
      sendNoArgsPacket(network.socket, CTOS_STILL_ONLINE);
      lastPacket = current;
    }
  }

  return NULL;
}

void dismantleNetwork(){

  if (network.socket != -1)
    sendNoArgsPacket(network.socket, CTOS_END_CONNECTION);

  if (network.serverInfo) {
    freeaddrinfo(network.serverInfo);
    network.serverInfo = NULL;
  }

  //No need to free network.server, as it came from network.serverinfo
  if (network.packet) {
    free(network.packet);
    network.packet = NULL;
  }

  if (network.socket != -1)
    shutdown(network.socket, 2);

  network.socket = -1;
}


int main(){

  pthread_t *thread = NULL;

  enum CLIENT_STAGE{
    BEGINNING,
    LOBBY,
    GAMES_RUNNING
  };

  enum KEY_STATE{
    KEY_DOWN,
    KEY_UP,
  }keyState;
  keyState = KEY_UP;

  enum CLIENT_STAGE stage = BEGINNING;
  char outputText[TOTAL_OUTPUT_TEXTS][MAX_OUTPUT_TEXT_LEN+1] = {{"Enter a username (1-9 characters)"},
                                                                {"Username is taken"},
                                                                {"Cannot connect to network. Try again."}};
  int outputTextPos = USERNAME_PROMPT;
  SDL_Rect outputTextRects[TOTAL_OUTPUT_TEXTS];
  struct Texture *outputTextTextures[TOTAL_OUTPUT_TEXTS];
  char username[MAX_USERNAME_SIZE+1] = {'\0'};
  SDL_Rect usernameTextRect;
  struct Texture *usernameTextTexture = setupTexture();
  int inputPos = 0;
  uintmax_t refreshPlayers = 0;

  char playersOnline[MAX_PLAYERS][MAX_USERNAME_SIZE];
  SDL_Rect playersRects[MAX_PLAYERS];
  SDL_Rect playersDisplayRect;
  struct Texture *playersTextures[MAX_PLAYERS];
  int numPlayersOnline = 0;

  //A structure that will store monitor information such as the width and height of the monitor
  SDL_DisplayMode monitorInformation;
  //The SDL_Window structure
  SDL_Window* window = NULL;
  //This program will work using texture-based rendering, so it will use SDL_Renderer instead of SDL_Surface
  SDL_Renderer* renderer = NULL;

  //Dimensions of the window
  int windowWidth, windowHeight;

  //The position of the top left corner of the window relative to the monitor
  int window_x, window_y;

  //Window title which will be shown on the window bar
  const char *windowTitle = "GNU C Coders- Chess";

  //The name of the true type font that will be used in the program
  const char *calibri = "Font/Calibri.ttf";
  TTF_Font *calibri18 = NULL;
  TTF_Font *calibri24 = NULL;
  struct Texture *userInputTexture = setupTexture();
  bool inputChanged = false;
  bool inputLoaded = false;


  const char *boardPath = "Images/board.png";
  struct Texture *whitePiecesPics[DISTINCT_PIECES];
  struct Texture *blackPiecesPics[DISTINCT_PIECES];
  SDL_Rect chessBoard;
  struct Texture *boardPic;

  SDL_Rect alertBox;
  SDL_Rect userInputRect;

  if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS) < 0){
    fprintf(stderr, "Failed to initialise SDL.\n");
    return EXIT_FAILURE;
  }

  //Gets information on the indexed monitor,which is stored in the monitorInformation structure
  SDL_GetDesktopDisplayMode(0, &monitorInformation);

  if (monitorInformation.w > maxWindowWidth)
    windowWidth = maxWindowWidth;

  else
    windowWidth = monitorInformation.w * dimensionsScale;

  if (monitorInformation.h > maxWindowHeight)
    windowHeight = maxWindowHeight;

  else
    windowHeight = monitorInformation.h * dimensionsScale;


  window_x = windowWidth * windowPos_x_scale;
  window_y = windowHeight * windowPos_y_scale;

  if (windowWidth > windowHeight){
    chessBoard.w =  windowHeight*chessBoardScale;
    chessBoard.h = windowHeight * chessBoardScale;
  }

  else{
    chessBoard.w = windowWidth * chessBoardScale;
    chessBoard.h = windowWidth * chessBoardScale;
  }

  chessBoard.x = chessBoard.w * chessBoardPos_x_scale;
  chessBoard.y = chessBoard.h * chessBoardPos_y_scale;

  alertBox.x = windowWidth * alertBoxPos_x_scale;
  alertBox.y = 0;
  alertBox.w = windowWidth * alertBoxWidth_scale;
  alertBox.h = windowHeight;

  playersDisplayRect.x = 0;
  playersDisplayRect.y = 0;
  playersDisplayRect.w = windowWidth * playersBoxWidth_scale;
  playersDisplayRect.h = windowHeight;

  window = SDL_CreateWindow(windowTitle, window_x, window_y, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
  if (!window){
    fprintf(stderr, "Failed to initialise window.\n");
    return EXIT_FAILURE;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
  if (!renderer){
    fprintf(stderr, "Failed to initialise SDL renderer: %s.\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  if (! (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)){
    fprintf(stderr, "Failed to initialise SDL PNG image support: %s\n", IMG_GetError());
    return EXIT_FAILURE;
  }

  if (TTF_Init() < 0){
    fprintf(stderr, "Failed to initialise TTF: %s\n", TTF_GetError());
    return EXIT_FAILURE;
  }

  if(!(calibri18 = TTF_OpenFont (calibri,18)) || !(calibri24 = TTF_OpenFont(calibri, 24))){
    fprintf(stderr, "Failed to load calibri font.\n");
    return EXIT_FAILURE;
  }

  if(!(boardPic = setupTexture())){
    fprintf(stderr, "Failed to initialise texture.\n");
    return EXIT_FAILURE;
  }

  if(!loadImage(boardPic, renderer, boardPath, false, 0, 0, 0)){
    fprintf(stderr, "Failed to load board pic.\n");
    return EXIT_FAILURE;
  }


  SDL_Color white = {255,255,255,255};


  for (int i = 0; i<TOTAL_OUTPUT_TEXTS; i++) {
    outputTextTextures[i] = setupTexture();
    if (!loadText(outputTextTextures[i], calibri24, outputText[i], white, renderer)) {
      fprintf(stderr, "Failed to load text.\n");
      return EXIT_FAILURE;
    }
    outputTextRects[i].w = outputTextTextures[i]->width;
    outputTextRects[i].h = outputTextTextures[i]->height;
    outputTextRects[i].x = alertBox.x;
    outputTextRects[i].y = windowHeight*usernamePos_y_scale;
  }



  for (int i = 0; i < DISTINCT_PIECES; i++){
    whitePiecesPics[i] = setupTexture();
    blackPiecesPics[i] = setupTexture();

    if (!loadImage(whitePiecesPics[i], renderer, whitePiecesPaths[i], false, 0, 0, 0)||
        !loadImage(blackPiecesPics[i], renderer, blackPiecesPaths[i], false, 0, 0, 0)){
      fprintf(stderr, "Failed to load a piece picture.\n");
      return EXIT_FAILURE;
    }
  }

  for (int i = 0; i < MAX_PLAYERS; i++){
    playersTextures[i] = setupTexture();
    playersRects[i].x = 0;
  }


  userInputRect.x = windowWidth * usernamePos_x_scale;
  userInputRect.y = windowHeight * usernamePos_y_scale;
  userInputRect.w = usernameWidth;
  userInputRect.h = usernameHeight;

  usernameTextRect.x = userInputRect.x+userInputPaddingx;
  usernameTextRect.y = userInputRect.y;


  bool endProgram = false;
  SDL_Event event;

  setupNetwork();
  int expectReceiveCount = 0;
  while (!endProgram){
    if (network.socket != -1){

      if (stage == LOBBY) {
        uintmax_t current = (uintmax_t) time(NULL);

        if (current - refreshPlayers >= CLIENT_REQUEST_PLAYERS_INTERVAL) {
          sendNoArgsPacket(network.socket, CTOS_GET_PLAYERS);
          refreshPlayers = current;
          expectReceiveCount++;
        }
      }

      if (expectReceiveCount > 0){
        recievePacket(&network.packet, network.socket);

        if(network.packet->type == STOC_LIST_OF_PLAYERS){
          expectReceiveCount--;
          numPlayersOnline = network.packet->argc;

          for (int i = 0; i<numPlayersOnline; i++){
            memset(playersOnline[i],0,MAX_USERNAME_SIZE);
            strcpy(playersOnline[i], network.packet->args[i]);
            if (!loadText(playersTextures[i], calibri24, playersOnline[i], white, renderer)){
              numPlayersOnline = 0; //Not true, but impossible to format, so try again later
              break;
            }

            if (i == 0)
              playersRects[i].y = playersBox_y_padding;

            else
              playersRects[i].y = playersRects[i-1].y + playersTextures[i-1]->height + playersBox_y_padding;

            playersRects[i].w = playersTextures[i]->width;
            playersRects[i].h = playersTextures[i]->height;
          }
        }
      }

    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    //Green
    SDL_SetRenderDrawColor(renderer, 51, 255, 51, 0);
    SDL_RenderFillRect(renderer, &alertBox);


    if (stage == BEGINNING){
      //Grey
      SDL_SetRenderDrawColor(renderer, 88, 88, 88, 0);
      SDL_RenderFillRect(renderer, &userInputRect);

      renderTexture(outputTextTextures[outputTextPos], renderer, &outputTextRects[outputTextPos], NULL);
      if (inputChanged){
        if( loadText(usernameTextTexture, calibri24, username, white, renderer)){
          inputLoaded = true;
          usernameTextRect.w = usernameTextTexture->width;
          usernameTextRect.h = usernameTextTexture->height;
        }else{
          inputLoaded = false;
        }
      }

      if(inputLoaded)
        renderTexture(usernameTextTexture, renderer, &usernameTextRect, NULL);
    }

    else if (stage == LOBBY){
      renderTexture(boardPic, renderer, &chessBoard, NULL);
      SDL_SetRenderDrawColor(renderer, 88, 88, 88, 0);
      SDL_RenderFillRect(renderer, &playersDisplayRect);

      for (int i = 0; i<numPlayersOnline; i++)
        renderTexture(playersTextures[i], renderer, &playersRects[i], NULL);

    }


    while (SDL_PollEvent(&event)!=0){
      //If connected to server should send message to quit
      if (event.type == SDL_QUIT){
        endProgram = true;
      }else if (event.type == SDL_KEYDOWN && stage == BEGINNING && keyState == KEY_UP) {

        keyState = KEY_DOWN;
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_BACKSPACE && inputPos>0){
          inputPos--;
          username[inputPos] = '\0';
          inputChanged = true;
        }else if (SDLK_a <= key && key <= SDLK_z && inputPos < MAX_USERNAME_SIZE) {
          const char *c = SDL_GetKeyName(key);
          username[inputPos++] = 'a' + (c[0] - 'A');
          username[inputPos] = '\0';
          inputChanged = true;
        }else if (key == SDLK_RETURN && inputPos > 0){ //Send username to server
          if (network.socket != -1){
            network.packet->type = CTOS_SEND_USERNAME;
            network.packet->argc = 1;
            strcpy(network.packet->args[0], username);
            if (sendPacket(network.packet, network.socket) == -1){
              outputTextPos = CANNOT_CONNECT; //Will try and send again
              continue;
            }
            printf("recieving next packet.\n");
            recievePacket(&network.packet, network.socket);
            printf("recieved packet. %d\n", network.packet->type);
            if (network.packet->type == STOC_USERNAME_TAKEN){
              outputTextPos = USER_NAME_TAKEN;
            }else if (network.packet->type == STOC_USERNAME_VALID){
              printf("username is valid.");
              stage = LOBBY;
              thread = (pthread_t *) malloc(sizeof(pthread_t));
              pthread_create(thread, NULL, sendPings, NULL);
            }
          } else{
            outputTextPos = CANNOT_CONNECT;
            setupNetwork();
          }

        }

      }else if (event.type == SDL_KEYUP){
        keyState = KEY_UP;
      }
    }

    SDL_RenderPresent (renderer);
  }


  dismantleNetwork();
  if (thread)
    pthread_join(*thread, NULL);

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  TTF_CloseFont(calibri18);
  TTF_CloseFont(calibri24);
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
