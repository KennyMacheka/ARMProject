//
// Created by kenny on 13/06/18.
//

#ifndef ARM11_35_CHESS_ENGINE_H
#define ARM11_35_CHESS_ENGINE_H
#include <stdint.h>
#include <stdbool.h>

#define BOARD_SIZE 8
#define INITIAL_PIECES 16
#define WHITE_PAWN_PROMOTION_ROW  7
#define BLACK_PAWN_PROMOTION_ROW  0
#define DISTINCT_PIECES 6

enum MATCH_STATUS{
  NOT_OVER,
  WHITE_WIN,
  BLACK_WIN,
  STALEMATE
};

//WHITE_CHECK means white colour is in check
enum CHECK_STATUS{
  NO_CHECK,
  WHITE_CHECK,
  BLACK_CHECK
};

enum PIECES{
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  BLANK
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
  int  numWhitePieces;
  int  numBlackPieces;
  enum MATCH_STATUS matchState;
  enum CHECK_STATUS checkState;
  int fiftyMoveCount;
  enum COLOUR fiftyMoveSync;
  bool enPassantAvailable;
  //Location of pawn that just double moved
  int pawnEnPassantRow;
  int pawnEnPassantCol;
  //Where to move to in the event of en passant
  int enPassantMoveToRow;
  int enPassantMoveToCol;
};

struct Move{
  bool isCastling;
  bool isEnPassant;

  int startRow;
  int startCol;

  int endRow;
  int endCol;

  int startRow2;
  int startCol2;

  int endRow2;
  int endCol2;

  struct Piece *piece;
  struct Piece *piece2;
  enum PIECES promotionPiece;
};

struct PossibleMoves{
  struct Move *moves;
  int numMoves;
};


struct Game *setupGame ();
bool requestMove (struct Game *game, struct Move *move);


#endif //ARM11_35_CHESS_ENGINE_H
