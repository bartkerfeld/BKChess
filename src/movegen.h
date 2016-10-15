#pragma once

#include "board.h"

#define NOMOVE 0

enum MoveType {
  Normal, EnPassant, Castle, Promotion
};

/*
 * Move Representation
 * <-------------- 32-bits -------------->
 * 0000 0000 0000 0000 0000 0000 0011 1111 Source Square
 * 0000 0000 0000 0000 0000 1111 1100 0000 Destination Square
 * 0000 0000 0000 0000 0011 0000 0000 0000 Type (Normal, EnPassant, Castle, Promotion)
 * 0000 0000 0000 0011 1100 0000 0000 0000 Castle Permissions (before move was played)
 * 0000 0000 1111 1100 0000 0000 0000 0000 EnPassant Sqaure (before move was played)
 * 0000 1111 0000 0000 0000 0000 0000 0000 Piece Captured on Move
 * 1111 0000 0000 0000 0000 0000 0000 0000 Piece Promoted to on Move
 */
#define FROM_SQUARE(x) (x & 0x3F)
#define TO_SQUARE(x) ((x >> 6) & 0x3F)
#define MOVE_TYPE(x) ((x >> 12) & 0x3)
#define CASTLE_PERMISSION(x) ((x >> 14) & 0xF)
#define ENPASSANT_SQUARE(x) ((x >> 18) & 0x3F)
#define PIECE_CAPTURED(x) ((x >> 24) & 0xF)
#define PIECE_PROMOTED(x) ((x >> 28) & 0xF)

#define MOVE(f, t, mt, cp, es, pc, pp) \
  ((f) | (t<<6) | (mt<<12) | (cp<<14) | ((es & 0x3F)<<18) | (pc<<24) | (pp<<28))


typedef struct {
  int move;
  int score;
} Move;

typedef struct {
  Move moves[MAX_GAME_MOVES];
  int count;
} Movelist;

class MoveGenerator {
public:
  static void init();
  static int parse_move(char *ch, Board &board);
  static void generate_moves(const Board &board, Movelist &list);
  static void generate_capture_moves(const Board &board, Movelist &list);
  static std::string get_move(int &move);
  static bool square_attacked(const Square &square, const Color &attacker_color,
                              const Board &board);
private:
  static void add_quiet_move(const Board &board, int move, Movelist &list);
  static void add_capture_move(const Board &board, int move, Movelist &list);
  static void add_enpassant_move(const Board &board, int move, Movelist &list);
  static void add_white_pawn_capture_move(const Board &board, const int from, const int to, 
                                          const int cap, Movelist &list);
  static void add_white_pawn_move(const Board &board, const int from, const int to, Movelist &list);
  static void add_black_pawn_capture_move(const Board &board, const int from, const int to, 
                                          const int cap, Movelist &list);
  static void add_black_pawn_move(const Board &board, const int from, const int to, Movelist &list);
private:
  static U64 pawn_capture_moves(const Square &sq, const Color &color,
                                const U64 &white_pieces, const U64 &black_pieces);
  static U64 pawn_quiet_moves(const Square &sq, const Color &c, const U64 &occupied);
  static U64 knight_moves(const Square &sq, const U64 &same_color);
  static U64 bishop_moves(const Square &sq, const U64 &occupied, const U64 &same_color);
  static U64 rook_moves(const Square &sq, const U64 &occupied, const U64 &same_color);
  static U64 queen_moves(const Square &sq, const U64 &occupied, const U64 &same_color);
  static U64 king_moves(const Square &sq, const U64 &same_color);
  static U64 get_nonslider_moves(const int &piece_type, const Square &sq, const U64 &same_color);
  static U64 get_slider_moves(const int &piece_type, const Square &sq, const U64 &occupied, 
                              const U64 &same_color);
private:
  static U64 pawn_attacks(U64 pawns, Color color);
  static U64 knight_attacks(U64 knights, U64 same_color);
  static U64 bishop_attacks(U64 bishops, U64 occupied, U64 same_color);
  static U64 rook_attacks(U64 rooks, U64 occupied, U64 same_color);
  static U64 queen_attacks(U64 queens, U64 occupied, U64 same_color);
  static U64 king_attacks(U64 king, U64 same_color);
};

extern Square int_to_square[64];
