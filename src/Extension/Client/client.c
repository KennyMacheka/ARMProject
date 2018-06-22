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
#define SENDING_GAME_REQUEST 3
#define MATCH_NOT_POSSIBLE 4
#define TOTAL_OUTPUT_TEXTS 5


#define WAITING_FOR_OPPONENT 0
#define ACCEPT_REQUEST 1
#define REJECT_REQUEST 2
#define YOUR_MOVE 3
#define OPPONENT_MOVE 4
#define TOTAL_GAME_OUTPUT_TEXTS 5

#define MAX_OUTPUT_TEXT_LEN 50


//Working directory is extension
void setupNetwork();
void dismantleNetwork();
void *sendPings (void *dummy);
struct matchesStruct *setupMatchStruct();
void freeMatchStruct (struct matchesStruct *match);
struct matchesStruct *getMatchByPos(int pos);
struct matchesStruct *getMatchById(int id);


static int numMatches = 0;
static int currentMatch = 0;  //Displaying output of a particular match
static struct matchesStruct *newestMatch = NULL;
static struct matchesStruct *head = NULL;
//Similar to the one in server.c, however different enough that they can be defined separately
struct matchesStruct{
  int gameId;
  char opponentName[MAX_USERNAME_SIZE];
  char youAndOpponentMsg[MAX_OUTPUT_TEXT_LEN];
  enum COLOUR colour;
  struct Game* game;
  struct matchesStruct *next;
  struct matchesStruct *prev;
  int outputType;
  struct Texture *outputOpponentName;
  struct Texture *gameOutput;
  struct Texture *gameOutput2;

  char *outputText;
  char *outputText2;

  SDL_Rect outputTextRect;
  SDL_Rect outputTextRect2;
  SDL_Rect outputOpponentNameRect;

  bool gameStarted;

};

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


struct matchesStruct *setupMatchStruct(){
  struct matchesStruct *match = (struct matchesStruct *) malloc(sizeof(struct matchesStruct));
  match->game = NULL;
  match->next = NULL;
  match->prev = NULL;
  match->gameOutput = NULL;
  match->gameOutput2 = NULL;
  match->outputText = NULL;
  match->outputText2 = NULL;
  match->outputOpponentName = NULL;
  match->gameId = -1;
  match->outputType = -1;
  match->gameStarted = false;

  strcat(match->youAndOpponentMsg, "You vs. ");

  return match;
}

void initialiseMatchStruct(int gameId, char *opponentName, int outputType){
  struct matchesStruct *newMatch = setupMatchStruct();
  newMatch->gameId = gameId;
  strcpy(newMatch->opponentName, opponentName);
  newMatch->next = NULL;
  newMatch->prev = newestMatch;
  newMatch->gameOutput = setupTexture();
  newMatch->gameOutput2 = setupTexture();
  newMatch->outputOpponentName = setupTexture();
  newMatch->outputType = outputType;
  strcat(newMatch->youAndOpponentMsg, opponentName);

  if (newestMatch)
    newestMatch->next = newMatch;

  else {
    head->next = newMatch;
    currentMatch = 1;
  }

  newestMatch = newMatch;
  numMatches++;
}

void freeMatchStruct (struct matchesStruct *match){
  match->gameId = -1;

  if (newestMatch == match) {
    newestMatch = match->prev;
    if (!newestMatch)
      head->next = NULL;
  }

  if (match->game)
    free(match->game);

  if (match->next)
    match->next->prev = match->prev;

  if (match->prev)
    match->prev->next = match->next;

  if (match->gameOutput)
    freeTextureStructure(match->gameOutput);

  numMatches--;
  free(match);
}

struct matchesStruct *getMatchByPos(int pos){

  if (pos == 0)
    return NULL;

  int i = 0;
  struct matchesStruct *match = head;
  for (; i<pos && match; i++, match = match->next){
  }

  return match;
}

struct matchesStruct *getMatchById(int id){

  for (struct matchesStruct *match = head->next; match != NULL; match = match->next){
    if(match->gameId == id)
      return match;
  }

  return NULL;
}

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
  network.status = getaddrinfo(HOST_NAME, PORT_STR, &network.hints, &network.serverInfo);

  if (network.status != 0){
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(network.status));
    dismantleNetwork();
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

  head = setupMatchStruct();
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

  enum MOUSE_STATE{
    MOUSE_DOWN,
    MOUSE_UP
  }mouseState;

  mouseState = MOUSE_DOWN;
  keyState = KEY_UP;
  int mousePlayerPos;
  bool tryingToChallenge = false;

  enum CLIENT_STAGE stage = BEGINNING;
  char outputText[TOTAL_OUTPUT_TEXTS][MAX_OUTPUT_TEXT_LEN+1] = {{"Enter a username (1-9 characters)"},
                                                                {"Username is taken"},
                                                                {"Cannot connect to server. Try again."},
                                                                {"Sending challenge"},
                                                                {"Match no longer possible."}};

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

  char gameOutput[TOTAL_GAME_OUTPUT_TEXTS][MAX_OUTPUT_TEXT_LEN+1] = {{"Waiting for opponent..."},
                                                                     {"Accept Request"},
                                                                     {"Reject Request"},
                                                                     {"Your Move"},
                                                                     {"Opponent's move"}};

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


  const char *boardPath = "Images/board_brown.png";
  struct Texture *whitePiecesPics[DISTINCT_PIECES];
  struct Texture *blackPiecesPics[DISTINCT_PIECES];
  SDL_Rect chessBoard;
  struct Texture *boardPic;

  SDL_Rect alertBox;
  SDL_Rect userInputRect;

  //To navigate different games
  struct Texture *leftArrow = setupTexture();
  struct Texture *rightArrow = setupTexture();
  SDL_Rect leftArrowRect;
  SDL_Rect rightArrowRect;
  const char *leftArrowPath = "Images/left_arrow.png";
  const char *rightArrowPath = "Images/right_arrow.png";

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
  SDL_Rect windowRect = {0, 0, windowWidth, windowHeight};

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

  if(!loadImage(leftArrow, renderer, leftArrowPath, false, 0, 0, 0)){
    fprintf(stderr, "Failed to load left arrow.\n");
    return EXIT_FAILURE;
  }

  if(!loadImage(rightArrow, renderer, rightArrowPath, false, 0, 0, 0)){
    fprintf(stderr, "Failed to load right arrow.\n");
    return EXIT_FAILURE;
  }


  SDL_Color white = {255,255,255,255};
  SDL_Color black = {0,0,0,0};


  for (int i = 0; i<TOTAL_OUTPUT_TEXTS; i++) {
    outputTextTextures[i] = setupTexture();
    if (!loadText(outputTextTextures[i], calibri18, outputText[i], white, renderer)) {
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

  rightArrowRect.x = chessBoard.x+chessBoard.w - arrowWidth;
  rightArrowRect.y = windowHeight- arrowHeight - arrowPadY;
  rightArrowRect.w = arrowWidth;
  rightArrowRect.h = arrowHeight;

  leftArrowRect.x = rightArrowRect.x - arrowWidth - leftArrowPadX;
  leftArrowRect.y = rightArrowRect.y;
  leftArrowRect.w = arrowWidth;
  leftArrowRect.h = arrowHeight;


  bool endProgram = false;
  SDL_Event event;
  bool piecePressed = false;

  setupNetwork();
  int expectReceiveCount = 0;
  SDL_Rect clickedPieceRect;
  struct Piece *clickedPieceStruct;
  bool clickedPiece = false;

  int pieceSize = chessBoard.w/BOARD_SIZE;

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
        if (network.packet->type == STOC_LIST_OF_PLAYERS){
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

          tryingToChallenge = false;
        } else if (network.packet->type == STOC_FORWARDING_CHALLENGE_REQUEST){
          expectReceiveCount--;
          int gameId = *((int *) (&network.packet->args[0][0]));
          char *opponentName = network.packet->args[1];

          initialiseMatchStruct(gameId, opponentName, WAITING_FOR_OPPONENT);

        } else if (network.packet->type == STOC_CHALLENGE_REQUEST){
          int gameId = *((int *) (&network.packet->args[0][0]));
          char *opponentName = network.packet->args[1];
          initialiseMatchStruct(gameId, opponentName, ACCEPT_REQUEST);
        }else if (network.packet->type == STOC_CANNOT_CHALLENGE_PLAYER || network.packet->type == STOC_OPPONENT_LEFT){
          if (network.packet->type == STOC_CANNOT_CHALLENGE_PLAYER)
            expectReceiveCount--;

          int gameId = *((int *) (&network.packet->args[0][0]));
          struct matchesStruct *match = getMatchById(gameId);
          if (match)
            freeMatchStruct(match);
          if (currentMatch > numMatches)
            currentMatch = numMatches;
          //If I continue to develop, add code here so green box says player went offline
        }else if (network.packet->type == STOC_GAME_STARTED){
          expectReceiveCount--;
          int gameId = *((int *) (&network.packet->args[0][0]));
          enum COLOUR colour = *((int *) (&network.packet->args[1][0]));
          struct matchesStruct *match = getMatchById(gameId);

          if(match){
            match->game = setupGame();
            match->colour = colour;
            if (colour == WHITE){
              match->outputType = YOUR_MOVE;
            }
            //Player will move
            else {
              match->outputType = OPPONENT_MOVE;
              expectReceiveCount++;
            }
          }
        }else if (network.packet->type == STOC_OPPONENT_MOVE){
          expectReceiveCount--;
          int gameId = *((int *) (&network.packet->args[0][0]));
          struct matchesStruct *match = getMatchById(gameId);
          if (match){
            struct Move move;
            int startRow = * ((int *) &network.packet->args[1][0]);
            int startCol = * ((int *) &network.packet->args[2][0]);
            int endRow = * ((int *) &network.packet->args[3][0]);
            int endCol = * ((int *) &network.packet->args[4][0]);
            if (network.packet->argc == CASTLING_MOVE_ARGC){
              int startRow2 = * ((int *) &network.packet->args[5][0]);
              int startCol2 = * ((int *) &network.packet->args[6][0]);
              int endRow2 = * ((int *) &network.packet->args[7][0]);
              int endCol2 = * ((int *) &network.packet->args[8][0]);
              move = setupMoveStructCastling(match->game, startRow, startCol, endRow, endCol,
                                             startRow2, startCol2, endRow2, endCol2);
            }else{
              bool isEnpassant = (bool) *((int *) &network.packet->args[5][0]);
              enum PIECES promotionPiece = * ((int *) &network.packet->args[6][0]);
              move = setupMovesStructPromotion(match->game, startRow, startCol, endRow, endCol, promotionPiece);
            }

            match->outputType = YOUR_MOVE;
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
      renderTexture(rightArrow, renderer, &rightArrowRect, NULL);
      renderTexture(leftArrow, renderer, &leftArrowRect, NULL);

      for (int i = 0; i<numPlayersOnline; i++)
        renderTexture(playersTextures[i], renderer, &playersRects[i], NULL);

      if(head->next && currentMatch > 0){
        struct matchesStruct *match = getMatchByPos(currentMatch);
        if (match){
          if (match->outputOpponentName->width == 0){
            if (loadText(match->outputOpponentName, calibri24, match->youAndOpponentMsg, white, renderer)){
              match->outputOpponentNameRect.x = chessBoard.x + outputOpponentNamePaddingx;
              match->outputOpponentNameRect.y = outputOpponenty;
              match->outputOpponentNameRect.w = match->outputOpponentName->width;
              match->outputOpponentNameRect.h = match->outputOpponentName->height;
            }
          }

          if (match->outputOpponentName->width != 0 && match->outputOpponentName->height != 0){
            renderTexture(match->outputOpponentName, renderer, &match->outputOpponentNameRect, NULL);
          }

          if (match->gameStarted){ //output board

          }

          switch(match->outputType){
            case WAITING_FOR_OPPONENT:
              if (loadText(match->gameOutput, calibri24, gameOutput[WAITING_FOR_OPPONENT], white, renderer)){
                match->outputTextRect.x = (chessBoard.x+chessBoard.w)*0.5;
                match->outputTextRect.y = windowHeight - match->gameOutput->height - 20;
                match->outputTextRect.w = match->gameOutput->width;
                match->outputTextRect.h = match->gameOutput->height;
                renderTexture(match->gameOutput, renderer, &match->outputTextRect, NULL);
              }
              break;

            case ACCEPT_REQUEST:
              if (loadText(match->gameOutput, calibri24, gameOutput[ACCEPT_REQUEST], white, renderer) &&
                  loadText(match->gameOutput2, calibri24, gameOutput[REJECT_REQUEST], white, renderer)){

                match->outputTextRect.x = chessBoard.x;
                match->outputTextRect.y = windowHeight - match->gameOutput->height - 20;
                match->outputTextRect.w = match->gameOutput->width;
                match->outputTextRect.h = match->gameOutput->height;

                match->outputTextRect2.x = match->outputTextRect.x + match->outputTextRect.w + 50;
                match->outputTextRect2.y = windowHeight - match->gameOutput->height - 20;
                match->outputTextRect2.w = match->gameOutput2->width;
                match->outputTextRect2.h = match->gameOutput2->height;

                renderTexture(match->gameOutput, renderer, &match->outputTextRect, NULL);
                renderTexture(match->gameOutput2, renderer, &match->outputTextRect2, NULL);
              }
              break;

            case YOUR_MOVE: case OPPONENT_MOVE: //Show pieces on board
              SDL_RenderSetViewport(renderer, &chessBoard);
              //If colour is black, we can load up pieces in same order as board
              int sizePiece = chessBoard.w/BOARD_SIZE;
              int end_x = chessBoard.w, end_y = chessBoard.h;
              int start, end, inc;

              if (match->colour == BLACK){
                start = 0;
                end = BOARD_SIZE;
                inc = 1;
              }else{
                start = 7;
                end = -1;
                inc = -1;
              }

              for (int row = start, y = 0; row != end; row+=inc, y += sizePiece){
                for (int col = 0, x = 0; col<BOARD_SIZE; col++, x += sizePiece){
                  struct Piece piece = match->game->board[row][col];
                  if (piece.piece != BLANK){
                    struct Texture *pieceTexture;
                    if (piece.colour == WHITE)
                      pieceTexture = whitePiecesPics[piece.piece];
                    else
                      pieceTexture = blackPiecesPics[piece.piece];

                    SDL_Rect pieceRect;
                    pieceRect.x = x;
                    pieceRect.y = y;
                    pieceRect.w = sizePiece;
                    pieceRect.h = sizePiece;

                    renderTexture(pieceTexture, renderer, &pieceRect, NULL);
                  }
                }
              }
              SDL_RenderSetViewport(renderer, &windowRect);

              if (loadText(match->gameOutput, calibri24, gameOutput[match->outputType], white, renderer)){
                match->outputTextRect.x = chessBoard.x;
                match->outputTextRect.y = windowHeight - match->gameOutput->height - 20;
                match->outputTextRect.w = match->gameOutput->width;
                match->outputTextRect.h = match->gameOutput->height;
                renderTexture(match->gameOutput, renderer, &match->outputTextRect, NULL);
              }

              break;

          }
        }
      }
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

            recievePacket(&network.packet, network.socket);

            if (network.packet->type == STOC_USERNAME_TAKEN){
              outputTextPos = USER_NAME_TAKEN;
            }else if (network.packet->type == STOC_USERNAME_VALID){
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
      }else if(event.type == SDL_MOUSEBUTTONDOWN){
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        for (int i = 0; i<numPlayersOnline; i++){
          if (mouse_inside(mouseX,mouseY, playersRects[i])){ //User is trying to challenge a player
            mouseState = MOUSE_DOWN;
            mousePlayerPos = i;
            tryingToChallenge = true;
            break;
          }
        }

        if (mouse_inside(mouseX, mouseY, leftArrowRect) && currentMatch >= 2)
          currentMatch--;

        if (mouse_inside(mouseX, mouseY, rightArrowRect) && (currentMatch+1) <= numMatches)
          currentMatch++;

        if (stage == LOBBY && currentMatch != 0){
          struct matchesStruct *match = getMatchByPos(currentMatch);
          if (match) {
            if (match->outputType == ACCEPT_REQUEST) {
              if (mouse_inside(mouseX, mouseY, match->outputTextRect)) { //Accept request
                sendOneArgIntPacket(network.socket, CTOS_ACCEPT_REQUEST, match->gameId);
                expectReceiveCount++;
              } else if (mouse_inside(mouseX, mouseY, match->outputTextRect2)) { //Reject
                sendOneArgIntPacket(network.socket, CTOS_REJECT_MATCH_REQUEST, match->gameId);
                freeMatchStruct(match);
                if (currentMatch > numMatches)
                  currentMatch = numMatches;
              }
            }

            if (match->outputType == YOUR_MOVE && clickedPiece == false){
              int mouse_x, mouse_y;
              SDL_GetMouseState(&mouse_x, &mouse_y);
              SDL_RenderSetViewport(renderer, &chessBoard);
              mouse_x -= chessBoard.x;
              mouse_y -= chessBoard.y;
              //Need to loop through all pieces on board and check if they have been clicked on
              struct Piece *pieces;
              int numPieces;
              if (match->colour == WHITE){
                pieces = match->game->whitePieces;
                numPieces = match->game->numWhitePieces;
              }else{
                pieces = match->game->blackPieces;
                numPieces = match->game->numBlackPieces;
              }


              for (int i = 0; i<numPieces; i++){
                int screenX, screenY;
                screenX = pieces[i].col*pieceSize;
                if (match->colour == BLACK)
                  screenY = pieces[i].row*pieceSize;
                else
                  screenY = (BOARD_SIZE-pieces[i].row-1)*pieceSize;

                SDL_Rect pieceRect = {screenX, screenY, pieceSize, pieceSize};
                if (mouse_inside(mouse_x, mouse_y, pieceRect)) {
                  clickedPieceRect = pieceRect;
                  clickedPieceStruct = &pieces[i];
                  clickedPiece = true;
                  break;
                }
              }
              SDL_RenderSetViewport(renderer, &windowRect);
            }else if (match->outputType == YOUR_MOVE && clickedPiece){
              clickedPiece = false;
              int mouse_x, mouse_y;
              SDL_GetMouseState(&mouse_x, &mouse_y);
              SDL_RenderSetViewport(renderer, &chessBoard);
              mouse_x -= chessBoard.x;
              mouse_y -= chessBoard.y;
              int start, end,inc;

              if (match->colour == BLACK){
                start = 0;
                end = BOARD_SIZE;
                inc = 1;
              }else{
                start = 7;
                end = -1;
                inc = -1;
              }

              for (int row = start, y = 0; row != end; row+=inc, y += pieceSize){
                for (int col = 0, x = 0; col<BOARD_SIZE; col++, x += pieceSize){
                  struct Piece piece = match->game->board[row][col];
                  int screenX, screenY;
                  screenX = piece.col*pieceSize;
                  if (match->colour == BLACK)
                    screenY = piece.row*pieceSize;
                  else
                    screenY = (BOARD_SIZE-piece.row-1)*pieceSize;

                  SDL_Rect pieceRect;
                  pieceRect.x = screenX;
                  pieceRect.y = screenY;
                  pieceRect.w = pieceSize;
                  pieceRect.h = pieceSize;

                  if (mouse_inside(mouse_x, mouse_y, pieceRect)){
                    struct PossibleMoves *moves = setupMovesStruct();
                    switch (clickedPieceStruct->piece){
                      case PAWN:
                        pawnMoves(match->game, clickedPieceStruct, moves);
                        break;

                      case BISHOP:
                        bishopMoves(match->game, clickedPieceStruct, moves);
                        break;
                      case KNIGHT:
                        knightMoves(match->game, clickedPieceStruct, moves);
                        break;
                      case ROOK:
                        rookMoves(match->game, clickedPieceStruct, moves);
                        break;

                      case QUEEN:
                        queenMoves(match->game, clickedPieceStruct, moves);
                        break;

                      case KING:
                        kingMoves(match->game, clickedPieceStruct, moves);
                        break;
                      default:
                        break;
                    }
                    filterPossibleMoves(match->game, moves);

                    for (int m= 0; m < moves->numMoves; m++){
                      if (moves->moves[m].endRow == row && moves->moves[m].endCol == col){
                        requestMove(match->game, &moves->moves[m]);
                        network.packet->type = CTOS_MOVE;
                        if (moves->moves[m].isCastling){
                          sendCastlingMove(network.socket, CTOS_MOVE, match->gameId, moves->moves[m]);
                        }else{
                          sendNormalMove(network.socket, CTOS_MOVE, match->gameId, moves->moves[m]);
                        }
                        match->outputType = OPPONENT_MOVE;
                        expectReceiveCount++;
                        break;
                      }
                    }
                  }

                }
              }

              SDL_RenderSetViewport(renderer, &windowRect);
            }
          }
        }

      }else if (event.type == SDL_MOUSEBUTTONUP && mouseState == MOUSE_DOWN){
        mouseState = MOUSE_UP;
        if (tryingToChallenge){ //need to send challenge request
          int mouseX, mouseY;
          SDL_GetMouseState(&mouseX, &mouseY);
          if (mouse_inside(mouseX, mouseY, playersRects[mousePlayerPos])) {
            network.packet->type = CTOS_CHALLENGE_PLAYER;
            strcpy(network.packet->args[0], playersOnline[mousePlayerPos]);
            network.packet->argc = 1;
            sendPacket(network.packet, network.socket);
            expectReceiveCount++;
          }
          tryingToChallenge = false;
        }
      }
    }

    SDL_RenderPresent (renderer);
  }


  dismantleNetwork();
  if (thread)
    pthread_join(*thread, NULL);

  freeMatchStruct(head);
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  TTF_CloseFont(calibri18);
  TTF_CloseFont(calibri24);
  TTF_Quit();
  SDL_Quit();

  return EXIT_SUCCESS;
}
