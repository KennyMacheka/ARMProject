//
// Created by kenny on 13/06/18.
//

#ifndef ARM11_35_CHESS_ENGINE_H
#define ARM11_35_CHESS_ENGINE_H
#include <stdint.h>
#include <stdbool.h>

#define BOARD_SIZE 8
#define INITIAL_PIECES 16

enum MATCH_STATUS{
  NOT_OVER,
  WHITE_WIN,
  BLACK_WIN,
  STALEMATE
};

enum PIECES{
  BLANK,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING
};

enum COLOUR{
  NO_COLOUR,
  BLACK,
  WHITE
};

struct Piece{
  enum PIECES piece;
  enum COLOUR colour;
  bool moved;
  int row;
  int col;
};

struct Game{

  struct Piece board[BOARD_SIZE][BOARD_SIZE];
  struct Piece whitePieces[INITIAL_PIECES];
  struct Piece blackPieces[INITIAL_PIECES];
  uint8_t  numWhitePieces;
  uint8_t  numBlackPieces;
  enum MATCH_STATUS matchState;
};

struct Move{
  int startRow;
  int startCol;

  int endRow;
  int endCol;
  struct Piece *piece;
  enum PIECE promotionPiece;
};

struct PossibleMoves{
  struct Move *moves;
  int numMoves;
};


struct Game *setupGame ();
bool requestMove (struct Game *game, struct Move *move);


#endif //ARM11_35_CHESS_ENGINE_H
