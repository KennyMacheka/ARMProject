//
// Created by kenny on 13/06/18.
//

#ifndef ARM11_35_CHESS_ENGINE_H
#define ARM11_35_CHESS_ENGINE_H
#include <stdint.h>
enum PIECES{
  PAWN,
  KNIGHT,
  BISHOP,
  QUEEN,
  KING
};

struct game{

  uint8_t board[8][8];
  uint8_t  numWhitePieces;
  uint8_t  numBlackpieces;

};


#endif //ARM11_35_CHESS_ENGINE_H
