//
// Created by kenny on 13/06/18.
//
#include <stdlib.h>
#include <assert.h>
#include "chess_engine.h"

struct PossibleMoves *kingMoves (struct Game *game, struct Piece *king);
struct PossibleMoves *pawnMoves (struct Game *game, struct Piece *pawn);
struct PossibleMoves *queenMoves (struct Game *game, struct Piece *queen);
struct PossibleMoves *rookMoves (struct Game *game, struct Piece *rook);
struct PossibleMoves *knightMoves (struct Game *game, struct Piece *knight);
struct PossibleMoves *bishopMoves (struct Game *game, struct Piece *bishop);

struct PossibleMoves *setupMovesStruct();
void addMove (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol);
struct PossibleMoves *filterPossibleMoves(struct Game *game, struct PossibleMoves *moves);
void getMovesColumn (struct Game *game, struct Piece *piece, struct PossibleMoves *moves);
void getMovesDiagonal (struct Game *game, struct Piece *piece, struct PossibleMoves *moves);


struct Game *setupGame (){
  struct Game *game = (struct Game *) malloc(sizeof(struct Game));
  game->gameOver = false;
  game->numBlackPieces = INITIAL_PIECES;
  game->numWhitePieces = INITIAL_PIECES;

}


bool makeMove (struct Game *game, struct Move *move){
  if (game->board[move->startRow][move->startCol].piece == BLANK)
    return false;

  if (isMoveValid(game, move)){


  }

  return false;


}

bool isMoveValid (struct Game *game, struct Move *move){
  /*This is a move to check player is making the right move.
    This wouldn't be needed for an AI for instance*/

  struct PossibleMoves *possibleMoves = NULL;

  switch(game->board[move->startRow][move->startCol].piece){
    case PAWN:
      possibleMoves = pawnMoves(game, move->piece);
      break;

    case KNIGHT:
      possibleMoves = knightMoves(game, move->piece);
      break;

    case BISHOP:
      possibleMoves = bishopMoves(game, move->piece);
      break;


    case ROOK:
      possibleMoves = rookMoves(game, move->piece);
      break;

    case QUEEN:
      possibleMoves = queenMoves(game, move->piece);
      break;

    case KING:
      possibleMoves = kingMoves(game, move->piece);
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

/*
    How to check a move is valid:
      1. Know the range of possible moves a piece can take before hand
          For instance, if piece is a white pawn it can move forward 1 (or 2 if it hasn't moved yet),
          or diagonal if there is a piece in the way. Moving forward for a white pawn corresponds to subtracting row by 1

      2. Check position is not occupied, or there is not a piece in the way on the way to that position
          (for instance a rook can go across the board but can't jump over pieces)

      3. Make the move, then check if it puts the piece's king in check. If it does, move is invalid

 */

struct PossibleMoves *pawnMoves (struct Game *game, struct Piece *pawn){
  /**
    Complete for black pawn, and do right diagonal.
    Recommended to look at this function first so you know how to add a move.

    In every function where you calculate possible moves, remember to check if out of bounds.
    This means row and column must be >= 0 and < BOARD_SIZE (8).
   */
  assert (pawn->piece == PAWN && (pawn->colour == BLACK || pawn->colour == WHITE));

  if (game->gameOver)
    return NULL;

  enum COLOUR enemy = pawn->colour == WHITE ? BLACK:WHITE;
  struct PossibleMoves *moves = setupMovesStruct();

  //We get all possible moves
  //Then at the end of the function we take each move in turn...
  //...and check to see if the king goes in check. If it does, then move is invalid and we filter it out

  if (pawn->colour == BLACK){

    //Move black piece forward by 1
    if (pawn->row -1 >= 0){
      if (game->board[pawn->row-1][pawn->col].colour == NO_COLOUR){
        addMove(moves, pawn, pawn->row-1, pawn->col);
      }
    }

    //Move black piece forward by 2 if it hasn't moved yet
    if (!pawn->moved){
      if (game->board[pawn->row-2][pawn->col].colour == NO_COLOUR){
        addMove(moves, pawn, pawn->row-2, pawn->col);
      }
    }

    //Left diagonal
    if (game->board[pawn->row-1][pawn->col+1].colour == enemy){
      addMove(moves, pawn, pawn->row-1, pawn->col+1);
    }

    //Right diagonal
    if (game->board[pawn->row-1][pawn->col-1].colour == enemy){
      addMove(moves, pawn, pawn->row-1, pawn->col-1);
    }

  } else {
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

    //right diagonal
    if (game->board[pawn->row+1][pawn->col+1].colour == enemy){
      addMove(moves, pawn, pawn->row+1, pawn->col+1);
    }

    //left diagonal
    if (game->board[pawn->row+1][pawn->col-1].colour == enemy){
      addMove(moves, pawn, pawn->row+1, pawn->col-1);
    }
  }

  return moves;

}

struct PossibleMoves *bishopMoves (struct Game *game, struct Piece *bishop){
  /**This is complete. Implement getMovesDiagonal*/
  assert (bishop->piece == BISHOP && (bishop->colour == WHITE || bishop->colour == BLACK));
  enum COLOUR enemy = bishop->colour == WHITE ? BLACK:WHITE;

  if (game->gameOver)
    return NULL;

  struct PossibleMoves *moves = setupMovesStruct();

  getMovesDiagonal(game, bishop, moves);

  return moves;

}

struct PossibleMoves *knightMoves (struct Game *game, struct Piece *knight){
  /**Complete. A knight can move:
        One step forward two steps left or right
        Two steps forward 1 step left or right
        One step backward two steps left or right
        Steps backward one step left or right
   */
  assert (knight->piece == KNIGHT);
  enum COLOUR enemy = knight->colour == WHITE ? BLACK:WHITE;

  if (game->gameOver)
    return NULL;

  struct PossibleMoves *moves = setupMovesStruct();

  return moves;

}

struct PossibleMoves *rookMoves (struct Game *game, struct Piece *rook){
  /**This is complete. Implement  getMovesColumn)*/
  assert (rook->piece == ROOK && (rook->colour == WHITE || rook->colour == BLACK));
  enum COLOUR enemy = rook->colour == WHITE ? BLACK:WHITE;
  if (game->gameOver)
    return NULL;

  struct PossibleMoves *moves = setupMovesStruct();
  getMovesColumn(game, rook, moves);

  return moves;

}

struct PossibleMoves *queenMoves (struct Game *game, struct Piece *queen){
  /**This is complete. Implement getMovesDiagonal and getMovesColumn)*/
  assert (queen->piece == QUEEN && (queen->colour == WHITE || queen->colour == BLACK) );
  enum COLOUR enemy = queen->colour == WHITE ? BLACK:WHITE;
  if (game->gameOver)
    return NULL;

  struct PossibleMoves *moves = setupMovesStruct();
  getMovesDiagonal(game, queen, moves);
  getMovesColumn(game, queen, moves);

  return moves;

}

struct PossibleMoves *kingMoves (struct Game *game, struct Piece *king){
  /*
    Complete. King can move 1 step in any direction. Don't count a move if friendly piece is on it
   */
  assert (king->piece == KING && (king->colour == WHITE || king->colour == BLACK));
  enum COLOUR enemy = king->colour == WHITE ? BLACK:WHITE;

  if (game->gameOver)
    return NULL;

  struct PossibleMoves *moves = setupMovesStruct();

  return moves;
}

struct PossibleMoves *setupMovesStruct(){
  struct PossibleMoves *moves = (struct PossibleMoves*) malloc(sizeof(struct PosssibleMoves));
  assert (moves);
  moves->moves = NULL;
  moves->numMoves= 0;

  return moves;
}

bool coordWithinBoard(int row, int col) {
  return row < BOARD_SIZE && row >= 0 && col < BOARD_SIZE && col >= 0;
}

void getMovesDiagonal (struct Game *game, struct Piece *piece, struct PossibleMoves *moves){

  /**This function is for calculating the possible moves a bishop or queen can make
     Make this how you want. You need to inspect top left diagonal, top right diagonal,
     bottom right diagonal and bottom left diagonal, E.G.

     X.....X
     .X...X.
     ..X.X..
     ...p...
     ..X.X..
     .X...X.
     X.....X

   Where x is labelled, you method for getting diagonals must be able to get that coordinat/
   Hint : if you use a for loop, make sure you stop upon encountering a piece as you can't jump over a piece, and
   if a location has an enemy piece you can count that last move; if it is a friendly piece don't count it
   * */
  assert ((piece->piece == BISHOP) || (piece->piece == QUEEN) && (pawn->colour == BLACK || pawn->colour == WHITE));

  if (game->gameOver)
    return NULL;

  enum COLOUR enemy = piece->colour == WHITE ? BLACK:WHITE;
  struct PossibleMoves *moves = setupMovesStruct();

  int row = piece->row;
  int col = piece->col;

  bool blocked_topLeft = 0;
  bool blocked_botLeft = 0;
  bool blocked_topRight = 0;
  bool blocked_botRight = 0;

  for (int n = 1; n <= BOARD_SIZE; n++) {
    if (!blocked_topLeft && coordWithinBoard(row+n, col+n)) {
      if (game->board[row+n][col+n].colour == NO_COLOUR) {
        addMove(moves, piece, row+n, col+n);
      } else {
        blocked_topLeft = 1;
        if (game->board[row+n][col+n],colour == enemy) {
          addMove(moves, piece, row+n, col+n);
        }
      }
    } else if (!blocked_botLeft && coordWithinBoard(row-n, col+n)) {
      if (game->board[row-n][col+n].colour == NO_COLOUR) {
        addMove(moves, piece, row-n, col+n);
      } else {
        blocked_botLeft = 1;
        if (game->board[row-n][col+n],colour == enemy) {
          addMove(moves, piece, row-n, col+n);
        }
      }
    } else if (!blocked_topRight && coordWithinBoard(row+n, col-n)) {
      if (game->board[row+n][col-n].colour == NO_COLOUR) {
        addMove(moves, piece, row+n, col-n);
      } else {
        blocked_topRight = 1;
        if (game->board[row+n][col-n],colour == enemy) {
          addMove(moves, piece, row+n, col-n);
        }
      }
    } else if (!blocked_botRight &&  coordWithinBoard(row-n, col-n)) {
      if (game->board[row-n][col-n].colour == NO_COLOUR) {
        addMove(moves, piece, row-n, col-n);
      } else {
        blocked_botRight = 1;
        if (game->board[row-n][col-n],colour == enemy) {
          addMove(moves, piece, row-n, col-n);
        }
      }
    }
  }

  return moves;
}

void getMovesColumn (struct Game *game, struct Piece *piece, struct PossibleMoves *moves){

  /**
    This is similar to getMovesDiagonal except it's for a rook and queen,
    and this one should be easier as you just need to get all possible moves along
    the same rows and columns
   */
  assert ((piece->piece == BISHOP) || (piece->piece == QUEEN) && (pawn->colour == BLACK || pawn->colour == WHITE));

  if (game->gameOver)
    return NULL;

  enum COLOUR enemy = piece->colour == WHITE ? BLACK:WHITE;
  struct PossibleMoves *moves = setupMovesStruct();

  int row = piece->row;
  int col = piece->col;

  bool blocked_top = 0;
  bool blocked_left = 0;
  bool blocked_right = 0;
  bool blocked_bot = 0;

  for (int n = 1; n <= BOARD_SIZE; n++) {
    if (!blocked_top && coordWithinBoard(row+n, col)) {
      if (game->board[row+n][col].colour == NO_COLOUR) {
        addMove(moves, piece, row+n, col);
      } else {
        blocked_top = 1;
        if (game->board[row+n][col],colour == enemy) {
          addMove(moves, piece, row+n, col);
        }
      }
    } else if (!blocked_left && coordWithinBoard(row, col+n)) {
      if (game->board[row][col+n].colour == NO_COLOUR) {
        addMove(moves, piece, row, col+n);
      } else {
        blocked_left = 1;
        if (game->board[row][col+n],colour == enemy) {
          addMove(moves, piece, row, col+n);
        }
      }
    } else if (!blocked_right && coordWithinBoard(row, col-n)) {
      if (game->board[row][col-n].colour == NO_COLOUR) {
        addMove(moves, piece, row, col-n);
      } else {
        blocked_right = 1;
        if (game->board[row][col-n],colour == enemy) {
          addMove(moves, piece, row, col-n);
        }
      }
    } else if (!blocked_bot &&  coordWithinBoard(row-n, col)) {
      if (game->board[row-n][col].colour == NO_COLOUR) {
        addMove(moves, piece, row-n, col);
      } else {
        blocked_bot = 1;
        if (game->board[row-n][col],colour == enemy) {
          addMove(moves, piece, row-n, col);
        }
      }
    }
  }

  return moves;
}

void addMove (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol){
  moves->moves = realloc(moves->moves, sizeof(struct Move)*(moves->numMoves+1));
  moves->moves[moves->numMoves].piece = piece;
  moves->moves[moves->numMoves].startRow = piece->row;
  moves->moves[moves->numMoves].startCol = piece->col;
  moves->moves[moves->numMoves].endRow = endRow;
  moves->moves[moves->numMoves].endCol = endCol;

}

struct PossibleMoves *filterPossibleMoves(struct Game *game, struct PossibleMoves *moves){

}
