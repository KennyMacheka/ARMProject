//
// Created by kenny on 13/06/18.
//
#define _GNU_SOURCE
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include "../Chess_Engine/chess_engine.h"
#include "../network_protocols.h"

const int maxWindowWidth = 3600;
const int maxWindowHeight = 3600;
const double dimensionsScale = 0.90;
const double windowPos_x_scale = 0.01;
const double windowPos_y_scale = 0.05;

int main(){

  int status;
  int mainSocket, serverSocket;
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
  status = getaddrinfo("raspberrypi", PORT_STR, &hints, &serverInfo);
  if (status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return EXIT_FAILURE;
  }

  printf("Recieved address info.\n");

  for(server = serverInfo; server != NULL; server = server->ai_next){

    mainSocket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
    if (mainSocket == -1) {
      printf("client socket error.\n");
      continue;
    }

    serverSocket = connect(mainSocket, server->ai_addr, server->ai_addrlen);
    if (serverSocket == -1){
      printf("Client connect error.\n");
      continue;
    }

    break;

  }
  freeaddrinfo(serverInfo);


  if (server == NULL){
    printf("Failed to establish socket and connection.\n");
    return EXIT_FAILURE;
  }

  char msg[20];
  printf("Attempting to recieve message: \n");
  if (recv(mainSocket, msg, 20, 0) == 0){
    fprintf(stderr,"Received nothing from server.\n");
    return EXIT_FAILURE;
  }

  printf("Message recieved from server: %s\n", msg);

  shutdown(mainSocket, 2);


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
  const char* windowTitle = "GNU C Coders- Chess";

  //The name of the true type font that will be used in the program
  const char* calibri = "C:\\Windows\\Fonts\\Calibri.ttf";

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
  }

  if (TTF_Init() < 0){
    fprintf(stderr, "Failed to initialise TTF: %s\n", TTF_GetError());
  }

  bool endProgram = false;
  SDL_Event event;

  while (!endProgram){
    while (SDL_PollEvent(&event)!=0){
      //If connected to server should send message to quit
      if (event.type == SDL_QUIT){
        endProgram = true;
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  //Call TTF_CloseFont(fontName) if I later load a font
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
