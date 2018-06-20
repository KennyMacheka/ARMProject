//
// Created by kenny on 13/06/18.
//
#include <stdlib.h>
#include <assert.h>
#include "chess_engine.h"

void kingMoves (struct Game *game, struct Piece *king, struct PossibleMoves *moves);
void pawnMoves (struct Game *game, struct Piece *pawn, struct PossibleMoves *moves);
void queenMoves (struct Game *game, struct Piece *queen, struct PossibleMoves *moves);
void rookMoves (struct Game *game, struct Piece *rook, struct PossibleMoves *moves);
void knightMoves (struct Game *game, struct Piece *knight, struct PossibleMoves *moves);
void bishopMoves(struct Game *game, struct Piece *bishop, struct PossibleMoves *moves);

struct PossibleMoves *setupMovesStruct();
void addMove (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol);
void addCastleMove (struct PossibleMoves *moves, struct Piece *king, struct Piece *rook,
                    int endRowKing, int endColKing, int endRowRook, int endColRook);
void addEnPassant (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol);

void getAllPossibleMoves (struct Game *game, enum COLOUR colour, struct PossibleMoves *moves);
void filterPossibleMoves(struct Game *game, struct PossibleMoves *moves);
void getMovesRowColumn(struct Game *game, struct Piece *piece, struct PossibleMoves *moves);
void getMovesDiagonal (struct Game *game, struct Piece *piece, struct PossibleMoves *moves);
void makeMove (struct Game *game, struct Move *move);
bool isMoveValid (struct Game *game, struct Move *move);
bool isKingInCheck (struct Game *game, enum COLOUR colour);

void freePossibleMoves (struct PossibleMoves *moves);

struct Game *setupGame (){
  struct Game *game = (struct Game *) malloc(sizeof(struct Game));
  game->matchState = NOT_OVER;
  game->checkState = NO_CHECK;
  game->numBlackPieces = INITIAL_PIECES;
  game->numWhitePieces = INITIAL_PIECES;
  game->fiftyMoveCount = 0;
  game->fiftyMoveSync = BLACK;
  game->enPassantAvailable = false;

  int whiteRow = 0;
  int blackRow = 7;

  struct Piece wR1 = {.piece = ROOK, .colour = WHITE, .moved = false, .row = whiteRow, .col = 0};
  struct Piece wN1 = {.piece = KNIGHT, .colour = WHITE, .moved = false, .row = whiteRow, .col = 1};
  struct Piece wB1 = {.piece = BISHOP, .colour = WHITE, .moved = false, .row = whiteRow, .col = 2};
  struct Piece wQ = {.piece = QUEEN, .colour = WHITE, .moved = false, .row = whiteRow, .col = 3};
  struct Piece wK = {.piece = KING, .colour = WHITE, .moved = false, .row = whiteRow, .col = 4};
  struct Piece wB2 = {.piece = BISHOP, .colour = WHITE, .moved = false, .row = whiteRow, .col = 5};
  struct Piece wN2 = {.piece = KNIGHT, .colour = WHITE, .moved = false, .row = whiteRow, .col = 6};
  struct Piece wR2 = {.piece = ROOK, .colour = WHITE, .moved = false, .row = whiteRow, .col = 7};

  struct Piece bR1 = {.piece = ROOK, .colour = WHITE, .moved = false, .row = blackRow, .col = 0};
  struct Piece bN1 = {.piece = KNIGHT, .colour = WHITE, .moved = false, .row = blackRow, .col = 1};
  struct Piece bB1 = {.piece = BISHOP, .colour = WHITE, .moved = false, .row = blackRow, .col = 2};
  struct Piece bQ = {.piece = QUEEN, .colour = WHITE, .moved = false, .row = blackRow, .col = 3};
  struct Piece bK = {.piece = KING, .colour = WHITE, .moved = false, .row = blackRow, .col = 4};
  struct Piece bB2 = {.piece = BISHOP, .colour = WHITE, .moved = false, .row = blackRow, .col = 5};
  struct Piece bN2 = {.piece = KNIGHT, .colour = WHITE, .moved = false, .row = blackRow, .col = 6};
  struct Piece bR2 = {.piece = ROOK, .colour = WHITE, .moved = false, .row = blackRow, .col = 7};

  //Add white pieces
  game->board[whiteRow][0]= wR1;
  game->board[whiteRow][1] = wN1;
  game->board[whiteRow][2] = wB1;
  game->board[whiteRow][3] = wQ;
  game->board[whiteRow][4] = wK;
  game->board[whiteRow][5] = wB2;
  game->board[whiteRow][6] = wN2;
  game->board[whiteRow][7] = wR2;

  //Add white pawns
  for (int i = 0; i < BOARD_SIZE; i++){
    game->board[whiteRow+1][i].piece = PAWN;
    game->board[whiteRow+1][i].colour = WHITE;
    game->board[whiteRow+1][i].moved = false;
    game->board[whiteRow+1][i].row = whiteRow+1;
    game->board[whiteRow+1][i].col = i;
  }

  //Add black pieces
  game->board[blackRow][0] = bR1;
  game->board[blackRow][1] = bN1;
  game->board[blackRow][2] = bB1;
  game->board[blackRow][3] = bQ;
  game->board[blackRow][4] = bK;
  game->board[blackRow][5] = bB2;
  game->board[blackRow][6] = bN2;
  game->board[blackRow][7] = bR2;

  //Add black pawns
  for (int i = 0; i < BOARD_SIZE; i++){
    game->board[blackRow-1][i].piece = PAWN;
    game->board[blackRow-1][i].colour = BLACK;
    game->board[blackRow-1][i].moved = false;
    game->board[blackRow-1][i].row = blackRow-1;
    game->board[blackRow-1][i].col = i;
  }

  for (int i = 2; i<6; i++){
    for (int j = 0; j<BOARD_SIZE; j++){
      game->board[i][j].piece = BLANK;
      game->board[i][j].colour = NO_COLOUR;
      game->board[i][j].moved = false;
      game->board[i][j].row = i;
      game->board[i][j].row = j;
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

struct Move setupMoveStruct (struct Game *game, int startRow, int startCol, int endRow, int endCol,
                            bool isEnpassant){
  struct Move move;
  move.startRow = startRow;
  move.startCol = startCol;
  move.endRow = endRow;
  move.endCol = endCol;
  move.isCastling = false;
  move.isEnPassant = false;

  struct Piece piece = game->board[startRow][startCol];
  struct Piece (*pieces)[16] = piece.colour == WHITE ? &game->whitePieces : &game->blackPieces;
  int numPieces = piece.colour == WHITE ? game->numWhitePieces : game->numBlackPieces;

  for (int i = 0; i<numPieces; i++){
    if ((*pieces)[i].row == startRow && (*pieces)[i].col == startCol){
      move.piece = &(*pieces)[i];
      break;
    }
  }

  return move;
}

struct Move setupMoveStructCastling (struct Game *game, int startRow1, int startCol1, int endRow1,
                                     int endCol1, int startRow2, int startCol2, int endRow2, int endCol2){
  //King is always first piece
  struct Move move1 = setupMoveStruct(game, startRow1, startCol1, endRow1, endCol1, false);
  struct Move move2 = setupMoveStruct(game, startRow2, startCol2, endRow2, endCol2, false);
  struct Move move = move1;
  move.isEnPassant = false;
  move.isCastling = true;

  move.startRow2 = move2.startRow;
  move.startCol2 = move2.startCol;
  move.endRow2 = move2.endRow;
  move.endCol2 = move2.endCol;
  move.piece2 = move2.piece;

  return move;
}

struct Move setupMovesStructPromotion (struct Game *game, int startRow, int startCol, int endRow, int endCol,
                                       enum PIECES promotionPiece){

  struct Move move = setupMoveStruct(game, startRow, startCol, endRow, endCol, false);
  move.promotionPiece = promotionPiece;
  move.promotionPiece = promotionPiece;
  return move;
}

void deepCopy (struct Game *copyFrom, struct Game *copyTo){
  copyTo->matchState = copyFrom->matchState;
  copyTo->checkState = copyFrom->checkState;
  copyTo->numBlackPieces = copyFrom->numBlackPieces;
  copyTo->numWhitePieces = copyFrom->numWhitePieces;
  copyTo->fiftyMoveCount = copyFrom->fiftyMoveCount;
  copyTo->fiftyMoveSync = copyFrom->fiftyMoveSync;
  copyTo->enPassantAvailable = copyFrom->enPassantAvailable;
  if(copyFrom->enPassantAvailable){
    copyTo->enPassantMoveToRow = copyFrom->enPassantMoveToRow;
    copyTo->enPassantMoveToCol = copyFrom->enPassantMoveToCol;
    copyTo->pawnEnPassantRow = copyFrom->pawnEnPassantRow;
    copyTo->pawnEnPassantCol = copyFrom->pawnEnPassantCol;
  }

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
  int *numEnemyPieces = move->piece->colour == WHITE ?
                                                  &game->numBlackPieces : &game->numWhitePieces;

  struct Piece *enemyArray = move->piece->colour == WHITE ?
                                                    game->blackPieces : game->whitePieces;

  if (game->board[move->endRow][move->endCol].colour == enemy || move->isEnPassant){
    game->fiftyMoveCount = 0;
    //Resync fifty move rule
    game->fiftyMoveSync = move->piece->colour;
    *numEnemyPieces = *numEnemyPieces - 1;
    bool foundPiece = false;
    int captureRow, captureCol;
    if (move->isEnPassant){
      captureRow = game->pawnEnPassantRow;
      captureCol = game->pawnEnPassantCol;
    }else{
      captureRow = move->endRow;
      captureCol = move->endCol;
    }

    //Move captured piece to the end of the array
    for (int i = 0; i<INITIAL_PIECES-1; i++){
      if (!foundPiece) {
        if (enemyArray[i].row == captureRow && enemyArray[i].col == captureCol) {
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

  game->enPassantAvailable = false;


  if (move->piece->piece == PAWN &&
      (move->endRow - move->piece->row == 2 || move->endRow - move->piece->row == -2)){
    game->enPassantAvailable = true;
    game->pawnEnPassantRow = move->endRow;
    game->pawnEnPassantCol = move->endCol;
    game->enPassantMoveToCol = move->endCol;

    if (move->piece->colour == WHITE)
      game->enPassantMoveToRow = move->endRow-1;

    else
      game->enPassantMoveToRow = move->endRow+1;
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

  if (move->isCastling){
    //rook
    move->piece2->row = move->endRow2;
    move->piece2->col = move->endCol2;
    game->board[move->endRow2][move->endRow2] = *move->piece2;
  }

  //Once a move has been made, we need to check if the king of the opposite colour is in check
  if (isKingInCheck(game, enemy))
    game->checkState = enemy == WHITE ? WHITE_CHECK : BLACK_CHECK;

  //We now need to check if the next player has any valid moves
  struct PossibleMoves *moves = setupMovesStruct();

  getAllPossibleMoves(game, enemy, moves);
  //Get only valid moves
  filterPossibleMoves(game, moves);

  //No valid moves - Stalemate or checkmate
  if (moves->numMoves == 0){
    if (game->checkState == NO_CHECK)
      game->matchState = STALEMATE;

    else if (game->checkState == WHITE_CHECK)
      game->matchState = BLACK_WIN;

    else if (game->checkState == BLACK_CHECK)
      game->matchState = WHITE_WIN;
  }
}

bool coordWithinBoard(int row, int col) {
  return row < BOARD_SIZE && row >= 0 && col < BOARD_SIZE && col >= 0;
}

bool isMoveValid (struct Game *game, struct Move *move){
  /*This is a move to check player is making the right move.
    This wouldn't be needed for an AI for instance*/

  struct PossibleMoves *possibleMoves = setupMovesStruct();
  bool valid = false;

  switch(game->board[move->startRow][move->startCol].piece){
    case PAWN:
      pawnMoves(game, move->piece, possibleMoves);
      break;

    case KNIGHT:
      knightMoves(game, move->piece, possibleMoves);
      break;

    case BISHOP:
      bishopMoves(game, move->piece, NULL);
      break;

    case ROOK:
      rookMoves(game, move->piece, possibleMoves);
      break;

    case QUEEN:
      queenMoves(game, move->piece, possibleMoves);
      break;

    case KING:
      kingMoves(game, move->piece, possibleMoves);
      break;

    case BLANK:
      return false;
  }

  assert (possibleMoves);
  filterPossibleMoves(game, possibleMoves);

  for (int i = 0; i < possibleMoves->numMoves; i++){
    if (possibleMoves->moves[i].endRow == move->endRow &&
        possibleMoves->moves[i].endCol == move->endCol) {
      valid = true;
    }

  }

  freePossibleMoves(possibleMoves);
  return valid;
}

void getAllPossibleMoves (struct Game *game, enum COLOUR colour, struct PossibleMoves *moves){
  assert( moves && (colour == WHITE || colour == BLACK));
  /**Returns all unfiltered moves a certain colour could potentially make.
    It's the job of the caller to distinguish between the ones that are invalid,
    as they result or maintain check, and which are valid
   */

  int numPieces = colour == WHITE ? game->numWhitePieces : game->numBlackPieces;
  struct Piece *pieces = colour == WHITE ? game->whitePieces : game->blackPieces;

  void (*moveFunctions[DISTINCT_PIECES])(struct Game *, struct Piece *,
                                         struct PossibleMoves *) = {[PAWN] = &pawnMoves,
                                        [KNIGHT] = &knightMoves,
                                        [BISHOP] = &bishopMoves,
                                        [QUEEN] = &queenMoves,
                                        [ROOK] = &rookMoves,
                                        [KING] = &kingMoves};
  for (int i = 0; i < numPieces; i++)
    (*moveFunctions[pieces[i].piece])(game, &pieces[i], moves);
}

bool isKingInCheck (struct Game *game, enum COLOUR colour) {
  /**Checks if king of colour is in check*/
  assert (colour == WHITE || colour == BLACK);

  int kingRow, kingCol;
  enum COLOUR enemy = colour == WHITE ? BLACK : WHITE;
  struct PossibleMoves *moves = setupMovesStruct();
  struct Piece *pieces = colour == WHITE ? game->whitePieces : game->blackPieces;
  int numPieces = colour == WHITE ? game->numWhitePieces : game->numBlackPieces;

  bool check = false;

  //Get all possible moves of opposite colour
  getAllPossibleMoves(game, enemy, moves);

  for (int i = 0; i < numPieces; i++){
    if (pieces[i].piece == KING){
      kingRow = pieces[i].row;
      kingCol = pieces[i].col;
      break;
    }
  }


  for (int i = 0; i<moves->numMoves; i++){
    if (moves->moves[i].endRow == kingRow && moves->moves[i].endCol == kingCol) {
      check = true;
      break;
    }
  }

  freePossibleMoves(moves);
  return check;

}

void pawnMoves (struct Game *game, struct Piece *pawn, struct PossibleMoves *moves){
  assert (moves && pawn->piece == PAWN && (pawn->colour == BLACK || pawn->colour == WHITE));

  if (game->matchState != NOT_OVER)
    return;

  enum COLOUR enemy = pawn->colour == WHITE ? BLACK:WHITE;


  //We get all possible moves
  //Then at the end of the function we take each move in turn...
  //...and check to see if the king goes in check. If it does, then move is invalid and we filter it out

  if (pawn->colour == BLACK && pawn->row-1 >= 0){
    if (game->enPassantAvailable){
      int diffCol = game->enPassantMoveToCol-pawn->col;
      if (game->enPassantMoveToRow == pawn->row-1 && (diffCol == 1 || diffCol == -1))
        addEnPassant(moves, pawn, game->enPassantMoveToRow, game->enPassantMoveToCol);
    }

    //Move black piece forward by 1
    if (game->board[pawn->row-1][pawn->col].colour == NO_COLOUR){
      addMove(moves, pawn, pawn->row-1, pawn->col);
    }

    //Move black piece forward by 2 if it hasn't moved yet
    if (!pawn->moved){
      if (game->board[pawn->row-2][pawn->col].colour == NO_COLOUR)
        addMove(moves, pawn, pawn->row-2, pawn->col);
    }
    //left diagonal
    if (pawn->col+1 < BOARD_SIZE) {
      if (game->board[pawn->row - 1][pawn->col + 1].colour == enemy)
        addMove(moves, pawn, pawn->row - 1, pawn->col + 1);

    }

    //right diagonal
    if (pawn->col-1 >= 0) {
      if (game->board[pawn->row - 1][pawn->col - 1].colour == enemy)
        addMove(moves, pawn, pawn->row - 1, pawn->col - 1);

    }

  } else if (pawn->colour == WHITE && pawn->row+1 < BOARD_SIZE) {

    if (game->enPassantAvailable){
      int diffCol = pawn->col - game->enPassantMoveToCol;

      if (pawn->row+1 == game->enPassantMoveToRow && (diffCol == 1 || diffCol == -1))
        addEnPassant(moves, pawn, game->enPassantMoveToRow, game->enPassantMoveToCol);
    }
    //Move white piece forward by 1
    if (pawn->row+1 < BOARD_SIZE){
      if (game->board[pawn->row+1][pawn->col].colour == NO_COLOUR){
        addMove(moves, pawn, pawn->row+1, pawn->col);
      }
    }

    //Move white piece forward by 2 if it hasn't moved yet
    if (!pawn->moved){
      if (game->board[pawn->row+2][pawn->col].colour == NO_COLOUR)
        addMove(moves, pawn, pawn->row+2, pawn->col);

    }

    //left diagonal
    if (pawn->col-1 >= 0) {
      if (game->board[pawn->row + 1][pawn->col - 1].colour == enemy)
        addMove(moves, pawn, pawn->row + 1, pawn->col + 1);

    }

    //right diagonal
    if(pawn->col+1 < BOARD_SIZE) {
      if (game->board[pawn->row + 1][pawn->col + 1].colour == enemy)
        addMove(moves, pawn, pawn->row + 1, pawn->col - 1);

    }
  }

}

void bishopMoves(struct Game *game, struct Piece *bishop, struct PossibleMoves *moves) {
  assert (moves && bishop->piece == BISHOP && (bishop->colour == WHITE || bishop->colour == BLACK));

  if (game->matchState != NOT_OVER)
    return;

  getMovesDiagonal(game, bishop, moves);
}

void  knightMoves (struct Game *game, struct Piece *knight, struct PossibleMoves *moves){
  /** A knight can move:
        One step forward two steps left or right
        Two steps forward 1 step left or right
        One step backward two steps left or right
        Steps backward one step left or right
   */
  assert (moves && knight->piece == KNIGHT && (knight->colour == WHITE || knight->colour == BLACK));

  if (game->matchState != NOT_OVER)
    return;

  for (int i = -2; i <= 2; i++){
    for (int j = -2; j <= 2; j++){
      if (i == j || i == 0 || j == 0 || !coordWithinBoard(knight->row+i, knight->col+j))
        continue;

      if (game->board[knight->row+i][knight->col+j].colour != knight->colour)
        addMove(moves, knight, knight->row+i, knight->col+j);
    }
  }
}

void rookMoves (struct Game *game, struct Piece *rook, struct PossibleMoves *moves){
  assert (moves && rook->piece == ROOK && (rook->colour == WHITE || rook->colour == BLACK));
  
  if (game->matchState != NOT_OVER)
    return;

  getMovesRowColumn(game, rook, moves);

}

void queenMoves (struct Game *game, struct Piece *queen, struct PossibleMoves *moves){
  assert (moves && queen->piece == QUEEN && (queen->colour == WHITE || queen->colour == BLACK) );

  if (game->matchState != NOT_OVER)
    return;

  getMovesDiagonal(game, queen, moves);
  getMovesRowColumn(game, queen, moves);

}

void kingMoves (struct Game *game, struct Piece *king, struct PossibleMoves *moves){
  assert (moves && king->piece == KING && (king->colour == WHITE || king->colour == BLACK));

  if (game->matchState != NOT_OVER)
    return;

  for(int i = -1; i <= 1; i++){
    for (int j = -1; j <= 1; j++){
      if ((i == 0 && j == 0) || !coordWithinBoard(king->row+i, king->col+j))
        continue;

      if (game->board[king->row+i][king->col+j].colour != king->colour)
        addMove(moves, king, king->row+i, king->col+j);
    }
  }

  //Add castling
  if (!king->moved && game->checkState == NO_CHECK){
    struct Piece *pieces = king->colour == WHITE ? game->whitePieces : game->blackPieces;
    int numPieces = king->colour == WHITE ? game->numWhitePieces : game->numBlackPieces;

    for (int i = 0; i<numPieces; i++){
      if (pieces[i].piece == ROOK && !pieces[i].moved && pieces[i].row == king->row){
        int newColKing;
        int newColRook;
        int increment;
        bool castleLegal = true;

        if (king->col < pieces[i].col){
          increment = 1;
          newColKing = king->col + 2;
          newColRook = pieces[i].col - 2;
        }else{
          increment = -1;
          newColKing = king->col - 2;
          newColRook = pieces[i].col + 3;
        }

        //Check no pieces are in way to prevent castle
        for ( int col = king->col + increment; col < pieces[i].col; col += increment){
          if (game->board[king->row][col].piece != BLANK)
            castleLegal = false;
        }

        if (castleLegal)
          addCastleMove(moves, king, &pieces[i], king->row, newColKing, king->row, newColRook);
      }
    }
  }
}

struct PossibleMoves *setupMovesStruct(){
  struct PossibleMoves *moves = (struct PossibleMoves*) malloc(sizeof(struct PossibleMoves));
  assert (moves);
  moves->moves = NULL;
  moves->numMoves= 0;

  return moves;
}


void getMovesDiagonal (struct Game *game, struct Piece *piece, struct PossibleMoves *moves){
  /**This function is for calculating the possible moves a bishop or queen can make*/
  assert ((piece->piece == BISHOP || piece->piece == QUEEN) &&
          (piece->colour == BLACK || piece->colour == WHITE));

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

  assert ((piece->piece == BISHOP || piece->piece == QUEEN) &&
          (piece->colour == BLACK || piece->colour == WHITE));

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

  bool promotePawn = false;
  int promotionPieces = 4;
  enum PIECES promotions[] = {ROOK, KNIGHT, QUEEN, BISHOP};
  int loopNum = 1;
  //Check for pawn promotion
  if (piece->piece == PAWN){
    if ((piece->colour == WHITE && endRow == WHITE_PAWN_PROMOTION_ROW)||
        (piece->colour == BLACK && endRow == BLACK_PAWN_PROMOTION_ROW)){
      promotePawn = true;
      loopNum = promotionPieces;
    }
  }

  moves->moves = realloc(moves->moves, sizeof(struct Move)*(moves->numMoves+loopNum));

  for (int i = 0; i < loopNum; i++) {
    moves->moves[moves->numMoves].isCastling = false;
    moves->moves[moves->numMoves].isEnPassant = false;
    moves->moves[moves->numMoves].piece = piece;
    moves->moves[moves->numMoves].startRow = piece->row;
    moves->moves[moves->numMoves].startCol = piece->col;
    moves->moves[moves->numMoves].endRow = endRow;
    moves->moves[moves->numMoves].endCol = endCol;

    if (promotePawn){
      moves->moves[moves->numMoves].promotionPiece = promotions[i];
    }

    moves->numMoves++;
  }
}

void addEnPassant (struct PossibleMoves *moves, struct Piece *piece, int endRow, int endCol){
  addMove(moves, piece, endRow, endCol);
  moves->moves[moves->numMoves-1].isEnPassant = true;
}

void addCastleMove (struct PossibleMoves *moves, struct Piece *king, struct Piece *rook,
                    int endRowKing, int endColKing, int endRowRook, int endColRook){

  assert(king->piece == KING && rook->piece == ROOK);

  moves-> moves = realloc(moves->moves, sizeof(struct Move)*(moves->numMoves+1));
  moves->moves[moves->numMoves].isCastling = true;
  moves->moves[moves->numMoves].isEnPassant = false;
  moves->moves[moves->numMoves].piece = king;
  moves->moves[moves->numMoves].piece = rook;

  moves->moves[moves->numMoves].startRow = king->row;
  moves->moves[moves->numMoves].startCol = king->col;

  moves->moves[moves->numMoves].startRow2 = rook->row;
  moves->moves[moves->numMoves].startCol2 = rook->col;

  moves->moves[moves->numMoves].endRow = endRowKing;
  moves->moves[moves->numMoves].endCol = endColKing;

  moves->moves[moves->numMoves].endRow2 = endRowRook;
  moves->moves[moves->numMoves].endCol2 = endColRook;
  moves->numMoves++;
}

void filterPossibleMoves(struct Game *game, struct PossibleMoves *moves){
  /**Loops through given moves and checks to see what moves put a King in check*/
  struct Move *validMoves = (struct Move *) malloc(sizeof(struct Move)*moves->numMoves);
  int numValidMoves = 0;

  for (int i = 0; i < moves->numMoves; i++){
    //Make a deep copy of game before making move
    struct Game copy;
    deepCopy(game, &copy);
    makeMove(&copy, &moves->moves[i]);
    if (!isKingInCheck(&copy,moves->moves[i].piece->colour)){
      validMoves[numValidMoves] = moves->moves[i];
      numValidMoves ++;
    }
  }

  free(moves->moves);
  moves->moves = validMoves;
  moves->numMoves = numValidMoves;
}

void freePossibleMoves (struct PossibleMoves *moves){
  if (moves->moves)
    free(moves->moves);

  free(moves);
}
