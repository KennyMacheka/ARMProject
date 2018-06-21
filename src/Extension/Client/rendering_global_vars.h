//
// Created by kenny on 18/06/18.
//

#ifndef ARM11_35_RENDERING_GLOBAL_VARS_H
#define ARM11_35_RENDERING_GLOBAL_VARS_H
#define PIECE_PATH_LEN 14

#include "../Chess_Engine/chess_engine.h"
#include "../network_protocols.h"

extern const int maxWindowWidth;
extern const int maxWindowHeight;
extern const double dimensionsScale;
extern const double windowPos_x_scale;
extern const double windowPos_y_scale;
extern const double chessBoardScale;
extern const double chessBoardPos_x_scale;
extern const double chessBoardPos_y_scale;
extern const double usernamePos_x_scale;
extern const double usernamePos_y_scale;
extern const int usernameWidth;
extern const int usernameHeight;
extern const double alertBoxWidth_scale;
extern const double alertBoxPos_x_scale;
extern const int userInputPaddingx;
extern const double playersBoxWidth_scale;
extern const int playersBoxSpacing;
extern const int playersBox_y_padding;


extern const char whitePiecesPaths[DISTINCT_PIECES][PIECE_PATH_LEN+1];
extern const char blackPiecesPaths[DISTINCT_PIECES][PIECE_PATH_LEN+1];

#endif //ARM11_35_RENDERING_GLOBAL_VARS_H
