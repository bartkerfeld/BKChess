#pragma once

#include <string>
#include "bitboard.h"

#define MAX_GAME_MOVES 2048
#define MAXDEPTH 64

enum Piece {
  None,
  White_Pawns, White_Knights, White_Bishops, White_Rooks, White_Queens, White_King,
  Black_Pawns, Black_Knights, Black_Bishops, Black_Rooks, Black_Queens, Black_King,
  White_Pieces, Black_Pieces, All_Pieces
};

enum CastlePerm {
  WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8
};

typedef struct {
  int move, castle_perm, enpassant, fifty_move;
  U64 position_key;
} Undo;

class Board {
public:
  void init();
  void reset();
  int parse_fen(char *fen);
  U64 pieces[16];
  U64 position_key;
  int enpassant, castle_perm, fifty_move, ply, history_ply;
  Color side;
  void print_board();
  Undo history[MAX_GAME_MOVES];
  int pv_array[MAXDEPTH];
  int search_history[13][64];
  int search_killers[2][MAXDEPTH];
private:
  void generate_position_key(Board board);
};

extern const Color piece_color[13];
