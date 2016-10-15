#include <iostream>
#include <cassert>

#include "movegen.h"
#include "makemove.h"
#include "zobrist.h"

#define HASH_PCE(pce, sq) (board.position_key ^= Zobrist::piece_keys[(pce)][(sq)])
#define HASH_CA (board.position_key ^= (Zobrist::castle_keys[(board.castle_perm)]))
#define HASH_SIDE (board.position_key ^= (Zobrist::side_key))
#define HASH_EP (board.position_key ^= (Zobrist::piece_keys[None][(board.enpassant)]))

static const int castle_perm[64] = {
  13, 15, 15, 15, 12, 15, 15, 14,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
   7, 15, 15, 15,  3, 15, 15, 11 
};

bool MoveMaker::make_move(Board &board, const int move) {
  int from = FROM_SQUARE(move);
  int to = TO_SQUARE(move);
  int side = board.side;

  assert(from >= 0 && from < 64);
  assert(to >= 0 && to < 64);

  board.history[board.history_ply].position_key = board.position_key;  

  if (MOVE_TYPE(move) == EnPassant) {
    if (side == White) {
      clear_piece(to - 8, board);
    }
    else {
      clear_piece(to + 8, board);
    }
  }
  else if (MOVE_TYPE(move) == Castle) {
    switch(to) {
      case C1:
        move_piece(A1, D1, board);
        break;
      case C8:
        move_piece(A8, D8, board);
        break;
      case G1:
        move_piece(H1, F1, board);
        break;
      case G8:
        move_piece(H8, F8, board);
        break;
      default:
        assert(false);
        break;
    }
  }  

  if (board.enpassant != NOSQ) HASH_EP;
  HASH_CA;

  board.history[board.history_ply].move = move;
  board.history[board.history_ply].fifty_move = board.fifty_move;
  board.history[board.history_ply].enpassant = board.enpassant;
  board.history[board.history_ply].castle_perm = board.castle_perm;

  board.castle_perm &= castle_perm[from];
  board.castle_perm &= castle_perm[to];
  board.enpassant = NOSQ;

  HASH_CA;
  
  board.fifty_move++;

  int captured = PIECE_CAPTURED(move);
  if (captured != None) {
    clear_piece(to, board);
    board.fifty_move = 0;
  }

  board.history_ply++;
  board.ply++;

  int king_index;
  int pawn_start_shift;
  int pawn_enpassant_shift;
  Piece side_pawns;
  if (side == White) {
    king_index = White_King;
    pawn_start_shift = 16;
    pawn_enpassant_shift = 8;
    side_pawns = White_Pawns;
  }
  else {
    king_index = Black_King;
    pawn_start_shift = -16;
    pawn_enpassant_shift = -8;
    side_pawns = Black_Pawns;
  }

  if (board.pieces[side_pawns] & (1ULL << from)) {
    board.fifty_move = 0;
    if (to == from + pawn_start_shift) {
      board.enpassant = from + pawn_enpassant_shift;
      HASH_EP;
    }
  }

  move_piece(from, to, board);

  int promoted_piece = PIECE_PROMOTED(move);
  if (promoted_piece != 0) {
    clear_piece(to, board);
    add_piece(to, board, promoted_piece);
  }

  board.side = (board.side == White) ? Black : White;
  HASH_SIDE;

  assert(board.pieces[king_index] != 0ULL);

  if (MoveGenerator::square_attacked(int_to_square[BitBoard::bit_scan_forward(board.pieces[king_index])], 
                                     board.side, board)) {
    take_move(board);
    return false;
  }
  
  return true;
}

void MoveMaker::take_move(Board &board) {
  assert(board.history_ply != 0 && board.ply != 0);

  board.history_ply--;
  board.ply--;

  int move = board.history[board.history_ply].move;
  int from = FROM_SQUARE(move);
  int to = TO_SQUARE(move);

  assert(from >= 0 && from < 64);
  assert(to >= 0 && to < 64);

  if (board.enpassant != NOSQ) HASH_EP;
  HASH_CA;

  board.castle_perm = board.history[board.history_ply].castle_perm;
  board.fifty_move = board.history[board.history_ply].fifty_move;
  board.enpassant = board.history[board.history_ply].enpassant;

  if (board.enpassant != NOSQ) HASH_EP;
  HASH_CA;

  board.side = (board.side == White) ? Black : White;
  HASH_SIDE;

  if (MOVE_TYPE(move) == EnPassant) {
    if (board.side == White) {
      add_piece(to - 8, board, Black_Pawns);
    }
    else {
      add_piece(to + 8, board, White_Pawns);
    }
  }
  else if (MOVE_TYPE(move) == Castle) {
    switch(to) {
      case C1:
        move_piece(D1, A1, board);
        break;
      case C8:
        move_piece(D8, A8, board);
        break;
      case G1:
        move_piece(F1, H1, board);
        break;
      case G8:
        move_piece(F8, H8, board);
        break;
      default:
        assert(false);
        break;
    }
  }

  move_piece(to, from, board);

  int captured = PIECE_CAPTURED(move);
  if (captured != 0) {
    if (MOVE_TYPE(move) != 1) {
      add_piece(to, board, captured);
    }
  }

  int promoted_piece = PIECE_PROMOTED(move);
  if (promoted_piece != None) {
    clear_piece(from, board);
    add_piece(from, board, (piece_color[promoted_piece] == White ? White_Pawns : Black_Pawns));
  }
}

void MoveMaker::clear_piece(const int sq, Board &board) {
  assert(board.pieces != NULL);

  U64 clear_mask = BitBoard::clear_mask[sq];
  for (int i = 0; i < 16; i++) {
    if (i >= 1 && i < 13 && (board.pieces[i] & (1ULL << sq))) {
      HASH_PCE(i, sq);
    }
    board.pieces[i] &= clear_mask;
  }
}

void MoveMaker::add_piece(const int sq, Board &board, const int piece) {
  assert(sq >= 0 && sq < 64);

  Piece side_pieces = (piece_color[piece] == White) ? White_Pieces : Black_Pieces;

  HASH_PCE(piece, sq);

  U64 mask = BitBoard::set_mask[sq];
  board.pieces[piece] |= mask;
  board.pieces[side_pieces] |= mask;
  board.pieces[All_Pieces] |= mask;
}

void MoveMaker::move_piece(const int from, const int to, Board &board) {
  assert(board.pieces != NULL);
  assert(from >= 0 && from < 64);
  assert(to >= 0 && to < 64);

  Piece side_pieces;

  U64 bb = 1ULL << from;
  int piece;
  for (piece = 1; piece < 13; piece++) {
    if (bb & board.pieces[piece]) {
      if (piece <= 6) {
        side_pieces = White_Pieces;
      }
      else {
        side_pieces = Black_Pieces;
      }
      break;
    }
  }

  assert(piece < 13);
  HASH_PCE(piece, from);
  HASH_PCE(piece, to);

  U64 clear_mask = BitBoard::clear_mask[from];
  board.pieces[piece] &= clear_mask;
  board.pieces[side_pieces] &= clear_mask;
  board.pieces[All_Pieces] &= clear_mask;

  U64 set_mask = BitBoard::set_mask[to];
  board.pieces[piece] |= set_mask;
  board.pieces[side_pieces] |= set_mask;
  board.pieces[All_Pieces] |= set_mask;
}

bool MoveMaker::move_exists(Board &board, const int move) {
  Movelist list;
  MoveGenerator::generate_moves(board, list);

  for (int move_num = 0; move_num < list.count; move_num++) {
    if (!make_move(board, list.moves[move_num].move)) {
      continue;
    }
    take_move(board);
    if (list.moves[move_num].move == move) {
      return true;
    }
  }

  return false;
}
