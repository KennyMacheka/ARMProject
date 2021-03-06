//
// Created by kenny on 18/06/18.
//

#include "rendering_global_vars.h"


const int maxWindowWidth = 3600;
const int maxWindowHeight = 3600;
const int usernameWidth = 300;
const int usernameHeight = 30;
const int userInputPaddingx = 1;
const double usernamePos_x_scale = 0.4;
const double usernamePos_y_scale = 0.5;
const double dimensionsScale = 0.90;
const double windowPos_x_scale = 0.1;
const double windowPos_y_scale = 0.05;
const double chessBoardScale = 0.75;
const double chessBoardPos_x_scale = 0.6;
const double chessBoardPos_y_scale = 0.1;
const double alertBoxWidth_scale = 0.25;
const double alertBoxPos_x_scale = 0.75;
const double playersBoxWidth_scale = 0.25;
const int playersBoxSpacing = 20;
const int playersBox_y_padding = 10;

const int outputOpponentNamePaddingx = 10;
const int outputOpponenty = 10;

const int arrowWidth = 50;
const int arrowHeight = 50;
const int leftArrowPadX = 10;
const int arrowPadY = 10;


const char whitePiecesPaths[DISTINCT_PIECES][PIECE_PATH_LEN+1] = {[PAWN] = "Images/wp.png",
    [KNIGHT] = "Images/wn.png", [BISHOP] = "Images/wb.png",
    [ROOK] = "Images/wr.png", [QUEEN] = "Images/wq.png",
    [KING] = "Images/wk.png"};

const char blackPiecesPaths[DISTINCT_PIECES][PIECE_PATH_LEN+1] = {[PAWN] = "Images/bp.png",
    [KNIGHT] = "Images/bn.png", [BISHOP] = "Images/bb.png",
    [ROOK] = "Images/br.png", [QUEEN] = "Images/bq.png",
    [KING] = "Images/bk.png"};


