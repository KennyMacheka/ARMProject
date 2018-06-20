//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <pthread.h>
#include "SDL_Libraries.h"
#include "sdl_utilities.h"
#include "../Chess_Engine/chess_engine.h"
#include "../network_protocols.h"
#include "rendering_global_vars.h"
#include "texture.h"

//Working directory is extension
void setupNetwork();
void dismantleNetwork();

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

void dismantleNetwork(){
  if (network.serverInfo)
    freeaddrinfo(network.serverInfo);
  //No need to free network.server, as it came from network.serverinfo
  if (network.packet)
    free(network.packet);

  if (network.socket != -1)
    shutdown(network.socket, 2);
}


int main(){

  enum CLIENT_STAGE{
    BEGINNING,
    LOBBY,
    GAMES_RUNNING
  };

  enum CLIENT_STAGE stage = LOBBY;
  char usernamePrompt[8+MAX_USERNAME_SIZE+2] = {'U','s','e','r','n','a','m','e',':',' ','\0'};

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
  struct Texture *userInputTexture = setupTexture();
  SDL_Rect userInputRect;

  const char *boardPath = "Images/board.png";
  struct Texture *whitePiecesPics[DISTINCT_PIECES];
  struct Texture *blackPiecesPics[DISTINCT_PIECES];
  SDL_Rect chessBoard;
  struct Texture *boardPic;

  SDL_Rect alertBox;

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
  alertBox.w = windowWidth* alertBoxWidth_scale;
  alertBox.h = windowHeight;

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

  if(!(calibri18 = TTF_OpenFont (calibri,18))){
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

  if(!loadText(userInputTexture, calibri18, usernamePrompt, white, renderer)){
    fprintf(stderr, "Failed to load text.\n");
    return EXIT_FAILURE;
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

  userInputRect.x = windowWidth * usernamePos_x_scale;
  userInputRect.y = windowHeight * usernamePos_y_scale;
  userInputRect.w = usernameWidth;
  userInputRect.h = usernameHeight;


  bool endProgram = false;
  SDL_Event event;
  setupNetwork();
  while (!endProgram){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    //SDL_SetRenderDrawColor(renderer, 51, 255, 51, 0);
    //SDL_RenderFillRect(renderer, &alertBox);


    if (stage == BEGINNING){
      //Grey
      SDL_SetRenderDrawColor(renderer, 88, 88, 88, 0);
      SDL_RenderFillRect(renderer, &userInputRect);
    }

    else{
      renderTexture(boardPic, renderer, &chessBoard, NULL);
    }

    while (SDL_PollEvent(&event)!=0){
      //If connected to server should send message to quit
      if (event.type == SDL_QUIT){
        endProgram = true;
      }
    }

    SDL_RenderPresent (renderer);
  }

  dismantleNetwork();
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  //Call TTF_CloseFont(fontName) if I later load a font
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
