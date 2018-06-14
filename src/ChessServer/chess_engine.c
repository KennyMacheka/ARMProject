//
// Created by kenny on 13/06/18.
//
#include <stdlib.h>
#include <assert.h>
#include "chess_engine.h"

struct PossibleMoves *kingMoves (struct Game *game, struct Piece *king, struct PossibleMoves *moves);
struct PossibleMoves *pawnMoves (struct Game *game, struct Piece *pawn, struct PossibleMoves *moves);
struct PossibleMoves *queenMoves (struct Game *game, struct Piece *queen, struct PossibleMoves *moves);
struct PossibleMoves *rookMoves (struct Game *game, struct Piece *rook, struct PossibleMoves *moves);
struct PossibleMoves *knightMoves (struct Game *game, struct Piece *knight, struct PossibleMoves *moves);
struct PossibleMoves *bishopMoves(struct Game *game, struct Piece *bishop, struct PossibleMoves *moves);

struct PossibleMoves *setupMovesStruct();
void addMove (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol);
struct PossibleMoves *filterPossibleMoves(struct Game *game, struct PossibleMoves *moves);
void getMovesRowColumn(struct Game *game, struct Piece *piece, struct PossibleMoves *moves);
void getMovesDiagonal (struct Game *game, struct Piece *piece, struct PossibleMoves *moves);
void makeMove (struct Game *game, struct Move *move);
bool isMoveValid (struct Game *game, struct Move *move);
bool isKingInCheck (struct Game *game, enum COLOUR colour);


struct Game *setupGame (){
  struct Game *game = (struct Game *) malloc(sizeof(struct Game));
  game->matchState = NOT_OVER;
  game->checkState = NO_CHECK;
  game->numBlackPieces = INITIAL_PIECES;
  game->numWhitePieces = INITIAL_PIECES;
  game->fiftyMoveCount = 0;
  game->fiftyMoveSync = BLACK;

  int whiteRow = 0;
  int blackRow = 7;
  int numPieces = 0;

  //Add white pieces
  game->board[whiteRow][0] = game->whitePieces[numPieces++] = {.piece = ROOK, .colour = WHITE, .moved = false, .row = whiteRow, .col = 0};
  game->board[whiteRow][1] = {.piece = KNIGHT, .colour = WHITE, .moved = false, .row = whiteRow, .col = 1};
  game->board[whiteRow][2] = {.piece = BISHOP, .colour = WHITE, .moved = false, .row = whiteRow, .col = 2};
  game->board[whiteRow][3] = {.piece = QUEEN, .colour = WHITE, .moved = false, .row = whiteRow, .col = 3};
  game->board[whiteRow][4] = {.piece = KING, .colour = WHITE, .moved = false, .row = whiteRow, .col = 4};
  game->board[whiteRow][5] = {.piece = BISHOP, .colour = WHITE, .moved = false, .row = whiteRow, .col = 5};
  game->board[whiteRow][6] = {.piece = KNIGHT, .colour = WHITE, .moved = false, .row = whiteRow, .col = 6};
  game->board[whiteRow][7] = {.piece = ROOK, .colour = WHITE, .moved = false, .row = whiteRow, .col = 7};

  //Add white pawns
  for (int i = 0; i < BOARD_SIZE; i++){
    game->board[whiteRow+1][i] = {.piece = PAWN, .colour = WHITE, .moved = false, .row = 1, .col = i};
  }

  //Add black pieces
  game->board[blackRow][0] = {.piece = ROOK, .colour = BLACK, .moved = false, .row = blackRow, .col = 0};
  game->board[blackRow][1] = {.piece = KNIGHT, .colour = BLACK, .moved = false, .row = blackRow, .col = 1};
  game->board[blackRow][2] = {.piece = BISHOP, .colour = BLACK, .moved = false, .row = blackRow, .col = 2};
  game->board[blackRow][3] = {.piece = QUEEN, .colour = BLACK, .moved = false, .row = blackRow, .col = 3};
  game->board[blackRow][4] = {.piece = KING, .colour = BLACK, .moved = false, .row = blackRow, .col = 4};
  game->board[blackRow][5] = {.piece = BISHOP, .colour = BLACK, .moved = false, .row = blackRow, .col = 5};
  game->board[blackRow][6] = {.piece = KNIGHT, .colour = BLACK, .moved = false, .row = blackRow, .col = 6};
  game->board[blackRow][7] = {.piece = ROOK, .colour = BLACK, .moved = false, .row = blackRow, .col = 7};

  //Add black pawns
  for (int i = 0; i < BOARD_SIZE; i++){
    game->board[blackRow-1][i] = {.piece = PAWN, .colour = BLACK, .moved = false, .row = blackRow-1, .col = i};
  }

  for (int i = 2; i<6; i++){
    for (int j = 0; j<BOARD_SIZE; j++){
      game->board[i][j] = {.piece = BLANK, .colour = NO_COLOUR, .moved = false, .row = i, .col = 0};
    }

  }

  //Add pieces to game->whitePieces and game->Blackpieces
  //The reason we have these structs is to easily find all pieces in the game
  //The alternative would be looping through the board which is a waste of time
  for (int i = 0; i<INITIAL_PIECES; i++) {
    game->whitePieces[i] = game->board[i/BOARD_SIZE][i % BOARD_SIZE];
    game->blackPieces[i] = game->board[BOARD_SIZE-(i/BOARD_SIZE)-1][i % BOARD_SIZE];

  }

  return game;
}

void deepCopy (struct Game *copyFrom, struct Game *copyTo){
  copyTo->matchState = copyFrom->matchState;
  copyTo->checkState = copyFrom->checkState;
  copyTo->numBlackPieces = copyFrom->numBlackPieces;
  copyTo->numWhitePieces = copyFrom->numWhitePieces;
  copyTo->fiftyMoveCount = copyFrom->fiftyMoveCount;
  copyTo->fiftyMoveSync = copyFrom->fiftyMoveSync;

  for (int i = 0; i<INITIAL_PIECES; i++) {
    copyTo->whitePieces[i] = copyFrom->whitePieces[i];
    copyTo->blackPieces[i] = copyFrom->blackPieces[i];
  }

  for (int i = 0; i<BOARD_SIZE; i++){
    for (int j = 0; j<BOARD_SIZE; j++)
      copyTo->board[i][j] = copyFrom->board[i][j];
  }


}


bool requestMove (struct Game *game, struct Move *move){
  if (game->board[move->startRow][move->startCol].piece == BLANK)
    return false;

  if (isMoveValid(game, move)){
    makeMove(game, move);
    return true;
  }

  return false;

}

void makeMove (struct Game *game, struct Move *move){
  //pre: move->piece must be a pointer to one of the pieces in the array of pieces
  //i.e. game->whitePieces and game->blackPieces

  enum COLOUR enemy = move->piece->colour == WHITE ? BLACK : WHITE;
  uint8_t *numEnemyPieces = move->piece->colour == WHITE ?
                                                  &game->numBlackPieces : &game->numWhitePieces;

  struct Piece *enemyArray = move->piece->colour == WHITE ?
                                                    game->blackPieces : game->whitePieces;


  if (game->board[move->endRow][move->endCol].colour == enemy){
    game->fiftyMoveCount = 0;
    //Resync fifty move rule
    game->fiftyMoveSync = move->piece->colour;
    *numEnemyPieces = *numEnemyPieces - (uint8_t )1;
    bool foundPiece = false;
    //Move captured piece to the end of the array
    for (int i = 0; i<INITIAL_PIECES-1; i++){
      if (!foundPiece) {
        if (enemyArray[i].row == move->endRow && enemyArray[i].col == move->endCol) {
          foundPiece = true;
        }
      }

      if (foundPiece){
        struct Piece temp = enemyArray[i];
        enemyArray[i] = enemyArray[i+1];
        enemyArray[i+1] = temp;
      }
    }
  }else{
    //Fifty move rule (no capture, and pawn hasn't been moved)
    if (move->piece->piece != PAWN && move->piece->colour == game->fiftyMoveSync)
      game->fiftyMoveCount++;
  }

  move->piece->row = move->endRow;
  move->piece->col = move->endCol;

  //Handle promotion
  if (move->piece->piece == PAWN){
    game->fiftyMoveCount = 0;
    if ((move->piece->colour == WHITE && move->endRow == WHITE_PAWN_PROMOTION_ROW) ||
        (move->piece->colour == BLACK && move->endRow == BLACK_PAWN_PROMOTION_ROW)){
      move->piece->piece = move->promotionPiece;
    }

  }

  game->board[move->endRow][move->endCol] = *move->piece;


  //Once a move has been made, we need to check if the king of the opposite colour is in check
  //To check for check we need to get all possible moves and check if one of them fits king's coordinates

  //If there aren't, game is over
  //We also check if there are any moves that can be made even if the opposite colour is not in check
  //If there aren't, then statemate
}

bool coordWithinBoard(int row, int col) {
  return row < BOARD_SIZE && row >= 0 && col < BOARD_SIZE && col >= 0;
}

bool isMoveValid (struct Game *game, struct Move *move){
  /*This is a move to check player is making the right move.
    This wouldn't be needed for an AI for instance*/

  struct PossibleMoves *possibleMoves = setupMovesStruct();

  switch(game->board[move->startRow][move->startCol].piece){
    case PAWN:
      pawnMoves(game, move->piece, possibleMoves);
      break;

    case KNIGHT:
      possibleMoves = knightMoves(game, move->piece, possibleMoves);
      break;

    case BISHOP:
      possibleMoves = bishopMoves(game, move->piece, NULL);
      break;

    case ROOK:
      possibleMoves = rookMoves(game, move->piece, possibleMoves);
      break;

    case QUEEN:
      possibleMoves = queenMoves(game, move->piece, possibleMoves);
      break;

    case KING:
      possibleMoves = kingMoves(game, move->piece, possibleMoves);
      break;
  }

  assert (possibleMoves);
  filterPossibleMoves(game, possibleMoves);

  for (int i = 0; i < possibleMoves->numMoves; i++){
    struct Move checkMoves = possibleMoves->moves[i];

    if (possibleMoves->moves[i].endRow == move->endRow &&
        possibleMoves->moves[i].endCol == move->endCol)
      return true;

  }

  return false;
}

bool isKingInCheck (struct Game *game, enum COLOUR colour) {
  /**Checks if king of colour is in check*/
  assert (colour == WHITE || colour == BLACK);

  struct PossibleMoves *moves = setupMovesStruct();
  struct Piece *pieces = colour == WHITE ? game->whitePieces : game->blackPieces;
  int numPieces = colour == WHITE ? game->numWhitePieces : game->numBlackPieces;
  struct Piece *enemyPieces = colour == WHITE ? game->blackPieces : game->whitePieces;
  int numEnemyPieces = colour == WHITE ? game->numWhitePieces : game->numBlackPieces;
  int kingRow, kingCol;

  void (*moveFunctions[DISTINCT_PIECES])(struct Game *, struct Piece *,
                                          struct PossibleMoves *) = {[PAWN] = &pawnMoves,
                                          [KNIGHT] = &knightMoves,
                                          [BISHOP] = &bishopMoves,
                                          [QUEEN] = &queenMoves,
                                          [ROOK] = &rookMoves,
                                          [KING] = &kingMoves};

  for (int i = 0; i < numPieces; i++){
    if (pieces[i].piece == KING){
      kingRow = pieces[i].row;
      kingCol = pieces[i].col;
      break;
    }
  }
  //Get all possible moves of pieces of opposite colour
  for (int i = 0; i < numEnemyPieces; i++){
    (*moveFunctions[enemyPieces[i].piece])(game, &enemyPieces[i], moves);
  }

  for (int i = 0; i<moves->numMoves; i++){
    if (moves->moves[i].endRow == kingRow && moves->moves[i].endCol == kingCol)
      return true;
  }

  return false;

}

struct PossibleMoves *pawnMoves (struct Game *game, struct Piece *pawn, struct PossibleMoves *moves){
  assert (moves && pawn->piece == PAWN && (pawn->colour == BLACK || pawn->colour == WHITE));

  if (game->matchState != NOT_OVER)
    return NULL;

  enum COLOUR enemy = pawn->colour == WHITE ? BLACK:WHITE;


  //We get all possible moves
  //Then at the end of the function we take each move in turn...
  //...and check to see if the king goes in check. If it does, then move is invalid and we filter it out

  if (pawn->colour == BLACK && pawn->row-1 >= 0){

    //Move black piece forward by 1

    if (game->board[pawn->row-1][pawn->col].colour == NO_COLOUR){
      addMove(moves, pawn, pawn->row-1, pawn->col);
    }

    //Move black piece forward by 2 if it hasn't moved yet
    if (!pawn->moved){
      if (game->board[pawn->row-2][pawn->col].colour == NO_COLOUR){
        addMove(moves, pawn, pawn->row-2, pawn->col);
      }
    }
    //left diagonal
    if (game->board[pawn->row-1][pawn->col+1].colour == enemy){
      addMove(moves, pawn, pawn->row-1, pawn->col+1);
    }

    //right diagonal
    if (game->board[pawn->row-1][pawn->col-1].colour == enemy){
      addMove(moves, pawn, pawn->row-1, pawn->col-1);
    }

  } else if (pawn->colour == WHITE && pawn->row+1 < BOARD_SIZE) {
    //Move white piece forward by 1
    if (pawn->row +1 < BOARD_SIZE){
      if (game->board[pawn->row+1][pawn->col].colour == NO_COLOUR){
        addMove(moves, pawn, pawn->row+1, pawn->col);
      }
    }

    //Move white piece forward by 2 if it hasn't moved yet
    if (!pawn->moved){
      if (game->board[pawn->row+2][pawn->col].colour == NO_COLOUR){
        addMove(moves, pawn, pawn->row+2, pawn->col);
      }
    }

    //left diagonal
    if (game->board[pawn->row+1][pawn->col-1].colour == enemy){
      addMove(moves, pawn, pawn->row+1, pawn->col+1);
    }

    //right diagonal
    if (game->board[pawn->row+1][pawn->col+1].colour == enemy){
      addMove(moves, pawn, pawn->row+1, pawn->col-1);
    }
  }

  return moves;

}

struct PossibleMoves *bishopMoves(struct Game *game, struct Piece *bishop, struct PossibleMoves *moves) {
  assert (moves && bishop->piece == BISHOP && (bishop->colour == WHITE || bishop->colour == BLACK));

  if (game->matchState != NOT_OVER)
    return NULL;

  getMovesDiagonal(game, bishop, moves);

  return moves;

}

struct PossibleMoves *knightMoves (struct Game *game, struct Piece *knight, struct PossibleMoves *moves){
  /** A knight can move:
        One step forward two steps left or right
        Two steps forward 1 step left or right
        One step backward two steps left or right
        Steps backward one step left or right
   */
  assert (moves && knight->piece == KNIGHT && (knight->colour == WHITE || knight->colour == BLACK));

  if (game->matchState != NOT_OVER)
    return NULL;

  enum COLOUR enemy = knight->colour == WHITE ? BLACK:WHITE;

  for (int i = -2; i <= 2; i++){
    for (int j = -2; j <= 2; j++){
      if (i == j || i == 0 || j == 0 || !coordWithinBoard(knight->row+i, knight->col+j))
        continue;

      if (game->board[knight->row+i][knight->col+j].colour != knight->colour)
        addMove(moves, knight, knight->row+i, knight->col+j);
    }
  }

  return moves;
}

struct PossibleMoves *rookMoves (struct Game *game, struct Piece *rook, struct PossibleMoves *moves){
  assert (moves && rook->piece == ROOK && (rook->colour == WHITE || rook->colour == BLACK));
  
  if (game->matchState != NOT_OVER)
    return NULL;

  getMovesRowColumn(game, rook, moves);

  return moves;

}

struct PossibleMoves *queenMoves (struct Game *game, struct Piece *queen, struct PossibleMoves *moves){
  assert (moves && queen->piece == QUEEN && (queen->colour == WHITE || queen->colour == BLACK) );

  enum COLOUR enemy = queen->colour == WHITE ? BLACK:WHITE;
  if (game->matchState != NOT_OVER)
    return NULL;

  getMovesDiagonal(game, queen, moves);
  getMovesRowColumn(game, queen, moves);

  return moves;

}

struct PossibleMoves *kingMoves (struct Game *game, struct Piece *king, struct PossibleMoves *moves){
  assert (moves && king->piece == KING && (king->colour == WHITE || king->colour == BLACK));
  enum COLOUR enemy = king->colour == WHITE ? BLACK:WHITE;

  if (game->matchState != NOT_OVER)
    return NULL;

  for(int i = -1; i <= 1; i++){
    for (int j = -1; j <= 1; j++){
      if ((i == 0 && j == 0 || !coordWithinBoard(king->row+i, king->col+j))
        continue;

      if (game->board[king->row+i][king->col+j].colour != king->colour)
        addMove(moves, king, king->row+i, king->col+j);
    }
  }

  //Add castling
  if (!king->moved && game->checkState == NO_CHECK){

  }
  
  return moves;
}

struct PossibleMoves *setupMovesStruct(){
  struct PossibleMoves *moves = (struct PossibleMoves*) malloc(sizeof(struct PosssibleMoves));
  assert (moves);
  moves->moves = NULL;
  moves->numMoves= 0;

  return moves;
}


void getMovesDiagonal (struct Game *game, struct Piece *piece, struct PossibleMoves *moves){
  /**This function is for calculating the possible moves a bishop or queen can make*/
  assert ((piece->piece == BISHOP) || (piece->piece == QUEEN) && (piece->colour == BLACK || piece->colour == WHITE));

  enum COLOUR enemy = piece->colour == WHITE ? BLACK:WHITE;
  moves = setupMovesStruct();

  int row = piece->row;
  int col = piece->col;

  bool blocked_topLeft = false;
  bool blocked_botLeft = false;
  bool blocked_topRight = false;
  bool blocked_botRight = false;

  for (int n = 1; n <= BOARD_SIZE; n++) {
    if (!blocked_topLeft && coordWithinBoard(row+n, col+n)) {
      if (game->board[row+n][col+n].colour == NO_COLOUR) {
        addMove(moves, piece, row+n, col+n);
      } else {
        blocked_topLeft = true;
        if (game->board[row+n][col+n].colour == enemy) {
          addMove(moves, piece, row+n, col+n);
        }
      }
    }
    if (!blocked_botLeft && coordWithinBoard(row-n, col+n)) {
      if (game->board[row-n][col+n].colour == NO_COLOUR) {
        addMove(moves, piece, row-n, col+n);
      } else {
        blocked_botLeft = true;
        if (game->board[row-n][col+n].colour == enemy) {
          addMove(moves, piece, row-n, col+n);
        }
      }
    }
    if (!blocked_topRight && coordWithinBoard(row+n, col-n)) {
      if (game->board[row+n][col-n].colour == NO_COLOUR) {
        addMove(moves, piece, row+n, col-n);
      } else {
        blocked_topRight = true;
        if (game->board[row+n][col-n].colour == enemy) {
          addMove(moves, piece, row+n, col-n);
        }
      }
    }
    if (!blocked_botRight &&  coordWithinBoard(row-n, col-n)) {
      if (game->board[row-n][col-n].colour == NO_COLOUR) {
        addMove(moves, piece, row-n, col-n);
      } else {
        blocked_botRight = true;
        if (game->board[row-n][col-n].colour == enemy) {
          addMove(moves, piece, row-n, col-n);
        }
      }
    }
  }

}

void getMovesRowColumn(struct Game *game, struct Piece *piece, struct PossibleMoves *moves){
  /**Gets moves along row and column*/

  assert ((piece->piece == BISHOP) || (piece->piece == QUEEN) && (piece->colour == BLACK || piece->colour == WHITE));

  enum COLOUR enemy = piece->colour == WHITE ? BLACK:WHITE;
  moves = setupMovesStruct();

  int row = piece->row;
  int col = piece->col;

  bool blocked_top = false;
  bool blocked_left = false;
  bool blocked_right = false;
  bool blocked_bot = false;

  for (int n = 1; n < BOARD_SIZE; n++) {
    if (!blocked_top && coordWithinBoard(row+n, col)) {
      if (game->board[row+n][col].colour == NO_COLOUR) {
        addMove(moves, piece, row+n, col);
      } else {
        blocked_top = true;
        if (game->board[row+n][col].colour == enemy) {
          addMove(moves, piece, row+n, col);
        }
      }
    }
    if (!blocked_left && coordWithinBoard(row, col+n)) {
      if (game->board[row][col+n].colour == NO_COLOUR) {
        addMove(moves, piece, row, col+n);
      } else {
        blocked_left = true;
        if (game->board[row][col+n].colour == enemy) {
          addMove(moves, piece, row, col+n);
        }
      }
    }
    if (!blocked_right && coordWithinBoard(row, col-n)) {
      if (game->board[row][col-n].colour == NO_COLOUR) {
        addMove(moves, piece, row, col-n);
      } else {
        blocked_right = true;
        if (game->board[row][col-n].colour == enemy) {
          addMove(moves, piece, row, col-n);
        }
      }
    }
    if (!blocked_bot &&  coordWithinBoard(row-n, col)) {
      if (game->board[row-n][col].colour == NO_COLOUR) {
        addMove(moves, piece, row-n, col);
      } else {
        blocked_bot = true;
        if (game->board[row-n][col].colour == enemy) {
          addMove(moves, piece, row-n, col);
        }
      }
    }

    if (blocked_bot && blocked_left && blocked_right && blocked_top)
      break;
  }

}

void addMove (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol){
  moves->moves = realloc(moves->moves, sizeof(struct Move)*(moves->numMoves+1));
  moves->moves[moves->numMoves].isCastling = false;
  moves->moves[moves->numMoves].piece = piece;
  moves->moves[moves->numMoves].startRow = piece->row;
  moves->moves[moves->numMoves].startCol = piece->col;
  moves->moves[moves->numMoves].endRow = endRow;
  moves->moves[moves->numMoves].endCol = endCol;
  moves->numMoves++;
}



struct PossibleMoves *filterPossibleMoves(struct Game *game, struct PossibleMoves *moves){

  /**Loops through given moves and checks to see what moves put a King in check*/
  for (int i = 0; i < moves->numMoves; i++){
    //Make a deep copy of game before making move
    struct Game *copy = setupGame();

    makeMove(game, &moves->moves[i]);
  }
}
