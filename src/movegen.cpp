#include <iostream>
#include <cassert>

#include "movegen.h"

static std::string square_name[64] = {
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

static std::string promotion_piece[4] = {
  "n", "b", "r", "q"
};

static std::string move_type[4] = {
  "normal", "enpassant", "castle", "promotion"
};

static const int victim_score[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int mvv_lva_scores[13][13];

Square int_to_square[64] = {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8
};

static const int LoopSlidePiece[8] = {
  White_Bishops, White_Rooks, White_Queens, 0,
  Black_Bishops, Black_Rooks, Black_Queens, 0
};
static const int LoopSlideIndex[2] = { 0, 4 };
static const int LoopNonSlidePiece[6] = {
  White_Knights, White_King, 0,
  Black_Knights, Black_King, 0
};
static const int LoopNonSlideIndex[2] = { 0, 3 };
static const int CaptureStartIndex[2] = { 7, 1 };
static const int CaptureEndIndex[2] = { 13, 7 };

void MoveGenerator::init() {
  for (int attacker = White_Pawns; attacker <= Black_King; attacker++) {
    for (int victim = White_Pawns; victim <= Black_King; victim++) {
      mvv_lva_scores[victim][attacker] = victim_score[victim] + 6 - ( victim_score[attacker] / 100);
    }
  }
}

int MoveGenerator::parse_move(char *ch, Board &board) {
  if (ch[1] > '8' || ch[1] < '1') return NOMOVE;
  if (ch[3] > '8' || ch[3] < '1') return NOMOVE;
  if (ch[0] > 'h' || ch[0] < 'a') return NOMOVE;
  if (ch[2] > 'h' || ch[2] < 'a') return NOMOVE;

  int from = (ch[0] - 'a') + (8 * (ch[1] - '1'));
  int to = (ch[2] - 'a') + (8 * (ch[3] - '1'));

  Movelist list;
  generate_moves(board, list);
  int move;
  for (int movenum = 0; movenum < list.count; movenum++) {
    move = list.moves[movenum].move;
    if (FROM_SQUARE(move) == from && TO_SQUARE(move) == to) {
      int promoted_piece = PIECE_PROMOTED(move);
      if (promoted_piece != None) {
        switch(promoted_piece) {
          case White_Queens:
          case Black_Queens:
            if (ch[4] == 'q') return move;
            break;
          case White_Rooks:
          case Black_Rooks:
            if (ch[4] == 'r') return move;
            break;
          case White_Bishops:
          case Black_Bishops:
            if (ch[4] == 'b') return move;
            break;
          case White_Knights:
          case Black_Knights:
            if (ch[4] == 'n') return move;
            break;
          default:
            assert(false);
        }
        continue;
      }

      return move;
    }
  }

  return NOMOVE;
}

std::string MoveGenerator::get_move(int &move) {
  std::string result = square_name[FROM_SQUARE(move)] + square_name[TO_SQUARE(move)];

  int promoted = PIECE_PROMOTED(move);
  if (promoted != 0) {
    result += promotion_piece[(promoted % 6) - 2];
  }

  //result += " " + move_type[MOVE_TYPE(move)];
  return result;
}

void MoveGenerator::generate_moves(const Board &board, Movelist &list) {
  list.count = 0;

  int from, to;
  U64 piece;
  U64 moves;
  int cap_piece;

  int (*bit_scan) (U64);

  if (board.side == White) {
    bit_scan = &BitBoard::bit_scan_forward;

    piece = board.pieces[White_Pawns];
    while (piece) {
      from = bit_scan(piece);
      moves = pawn_quiet_moves(int_to_square[from], board.side, board.pieces[All_Pieces]);

      while (moves) {
        to = bit_scan(moves);
        add_white_pawn_move(board, from, to, list);
        moves &= BitBoard::clear_mask[to];
      }

      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 board.pieces[White_Pieces], board.pieces[Black_Pieces]);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 7; cap_piece < 13; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << to)) {
            break;
          }
        }
        add_white_pawn_capture_move(board, from, to, cap_piece, list);
        moves &= BitBoard::clear_mask[to];
      }

      U64 enpassant = (board.enpassant == NOSQ) ? 0ULL : 1ULL << board.enpassant;
      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 board.pieces[White_Pieces], enpassant);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 7; cap_piece < 13; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << (to - 8))) {
            break;
          }
        }
        add_enpassant_move(board, MOVE(from, to, EnPassant, board.castle_perm, board.enpassant, cap_piece, None), list);
        moves &= BitBoard::clear_mask[to];
      }

      piece &= BitBoard::clear_mask[from];
    }

    if (board.castle_perm & WKCA) {
      U64 f1g1 = (1ULL << F1) | (1ULL << G1);
      if (!(board.pieces[All_Pieces] & f1g1)) {
        if (!square_attacked(E1, Black, board) & !square_attacked(F1, Black, board)) {
          add_quiet_move(board, MOVE(E1, G1, Castle, board.castle_perm, board.enpassant, None, None), list);
        }
      }
    }

    if (board.castle_perm & WQCA) {
      U64 b1c1d1 = (1ULL << B1) | (1ULL << C1) | (1ULL << D1);
      if (!(board.pieces[All_Pieces] & b1c1d1)) {
        if (!square_attacked(E1, Black, board) & !square_attacked(D1, Black, board)) {
          add_quiet_move(board, MOVE(E1, C1, Castle, board.castle_perm, board.enpassant, None, None), list);
        }
      }
    }

  }
  else {
    bit_scan = &BitBoard::bit_scan_reverse;
    piece = board.pieces[Black_Pawns];
    while (piece) {
      from = bit_scan(piece);
      moves = pawn_quiet_moves(int_to_square[from], board.side, board.pieces[All_Pieces]);

      while (moves) {
        to = bit_scan(moves);
        add_black_pawn_move(board, from, to, list);
        moves &= BitBoard::clear_mask[to];
      }

      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 board.pieces[White_Pieces], board.pieces[Black_Pieces]);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 1; cap_piece < 7; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << to)) {
            break;
          }
        }
        add_black_pawn_capture_move(board, from, to, cap_piece, list);
        moves &= BitBoard::clear_mask[to];
      }

      U64 enpassant = (board.enpassant == NOSQ) ? 0ULL : 1ULL << board.enpassant;
      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 enpassant, board.pieces[Black_Pieces]);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 1; cap_piece < 7; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << (to + 8))) {
            break;
          }
        }
        add_enpassant_move(board, MOVE(from, to, EnPassant, board.castle_perm, board.enpassant, cap_piece, None), list);
        moves &= BitBoard::clear_mask[to];
      }

      piece &= BitBoard::clear_mask[from];
    }

    if (board.castle_perm & BKCA) {
      U64 f8g8 = (1ULL << F8) | (1ULL << G8);
      if (!(board.pieces[All_Pieces] & f8g8)) {
        if (!square_attacked(E8, White, board) & !square_attacked(F8, White, board)) {
          add_quiet_move(board, MOVE(E8, G8, Castle, board.castle_perm, board.enpassant, None, None), list);
        }
      }
    }

    if (board.castle_perm & BQCA) {
      U64 b8c8d8 = (1ULL << B8) | (1ULL << C8) | (1ULL << D8);
      if (!(board.pieces[All_Pieces] & b8c8d8)) {
        if (!square_attacked(E8, White, board) & !square_attacked(D8, White, board)) {
          add_quiet_move(board, MOVE(E8, C8, Castle, board.castle_perm, board.enpassant, None, None), list);
        }
      }
    }
  }

  U64 side_pieces = (board.side == White) ? board.pieces[White_Pieces] : board.pieces[Black_Pieces];
  U64 opp_pieces = (board.side == White) ? board.pieces[Black_Pieces] : board.pieces[White_Pieces];
  int piece_type;

  // Non-Sliding Pieces
  int slide_idx = LoopNonSlideIndex[board.side];
  while (piece_type = LoopNonSlidePiece[slide_idx++]) {
    piece = board.pieces[piece_type];
    while (piece) {
      from = bit_scan(piece);
      moves = get_nonslider_moves(piece_type, int_to_square[from], side_pieces);
      while (moves) {
        to = bit_scan(moves);
        if ((1ULL << to) & opp_pieces) {
          for (cap_piece = CaptureStartIndex[board.side]; cap_piece < CaptureEndIndex[board.side]; cap_piece++) {
            if (board.pieces[cap_piece] & (1ULL << to)) {
              break;
            }
          }
          add_capture_move(board, MOVE(from, to, Normal, board.castle_perm, board.enpassant, cap_piece, None), list);
        }
        else {
          add_quiet_move(board, MOVE(from, to, Normal, board.castle_perm, board.enpassant, None, None), list);
        }
        moves &= BitBoard::clear_mask[to];
      }
      piece &= BitBoard::clear_mask[from];
    }
  }

  // Sliding Pieces
  slide_idx = LoopSlideIndex[board.side];
  while (piece_type = LoopSlidePiece[slide_idx++]) {
    piece = board.pieces[piece_type];
    while (piece) {
      from = bit_scan(piece);
      moves = get_slider_moves(piece_type, int_to_square[from], board.pieces[All_Pieces], side_pieces);
      while (moves) {
        to = bit_scan(moves);
        if ((1ULL << to) & opp_pieces) {
          for (cap_piece = CaptureStartIndex[board.side]; cap_piece < CaptureEndIndex[board.side]; cap_piece++) {
            if (board.pieces[cap_piece] & (1ULL << to)) {
              break;
            }
          }
          add_capture_move(board, MOVE(from, to, Normal, board.castle_perm, board.enpassant, cap_piece, None), list);
        }
        else {
          add_quiet_move(board, MOVE(from, to, Normal, board.castle_perm, board.enpassant, None, None), list);
        }
        moves &= BitBoard::clear_mask[to];
      }
      piece &= BitBoard::clear_mask[from];
    }
  }

}

bool MoveGenerator::square_attacked(const Square &square, const Color &attacker_color, const Board &board) {
  U64 bb_square = 1ULL << square;

  U64 same_color = (attacker_color == White) ? board.pieces[White_Pieces] : board.pieces[Black_Pieces];

  int start_index = (attacker_color == White) ? 1 : 7;

  U64 attacks =
    pawn_attacks(board.pieces[start_index], attacker_color) |
    knight_attacks(board.pieces[start_index + 1], same_color) |
    bishop_attacks(board.pieces[start_index + 2], board.pieces[All_Pieces], same_color) |
    rook_attacks(board.pieces[start_index + 3], board.pieces[All_Pieces], same_color) |
    queen_attacks(board.pieces[start_index + 4], board.pieces[All_Pieces], same_color) |
    king_attacks(board.pieces[start_index + 5], same_color);

  return bb_square & attacks;
}

void MoveGenerator::add_quiet_move(const Board &board, int move, Movelist &list) {
  list.moves[list.count].move = move;

  if (board.search_killers[0][board.ply] == move) {
    list.moves[list.count].score = 900000;
  }
  else if (board.search_killers[1][board.ply] == move) {
    list.moves[list.count].score = 800000;
  }
  else {
    int from = FROM_SQUARE(move);
    U64 from_bitboard = 1ULL << from;
    int piece_type;
    for (piece_type = White_Pawns; piece_type <= Black_King; piece_type++) {
      if (board.pieces[piece_type] & from_bitboard) {
        break; 
      }
    }
    assert(piece_type != 13);

    list.moves[list.count].score = board.search_history[piece_type][TO_SQUARE(move)];
  }

  list.count++;
}

void MoveGenerator::generate_capture_moves(const Board &board, Movelist &list) {
  list.count = 0;

  int from, to;
  U64 piece;
  U64 moves;
  int cap_piece;

  int (*bit_scan) (U64);

  if (board.side == White) {
    bit_scan = &BitBoard::bit_scan_forward;

    piece = board.pieces[White_Pawns];
    while (piece) {
      from = bit_scan(piece);

      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 board.pieces[White_Pieces], board.pieces[Black_Pieces]);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 7; cap_piece < 13; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << to)) {
            break;
          }
        }
        add_white_pawn_capture_move(board, from, to, cap_piece, list);
        moves &= BitBoard::clear_mask[to];
      }

      U64 enpassant = (board.enpassant == NOSQ) ? 0ULL : 1ULL << board.enpassant;
      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 board.pieces[White_Pieces], enpassant);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 7; cap_piece < 13; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << (to - 8))) {
            break;
          }
        }
        add_enpassant_move(board, MOVE(from, to, EnPassant, board.castle_perm, board.enpassant, cap_piece, None), list);
        moves &= BitBoard::clear_mask[to];
      }

      piece &= BitBoard::clear_mask[from];
    }
  }
  else {
    bit_scan = &BitBoard::bit_scan_reverse;
    piece = board.pieces[Black_Pawns];
    while (piece) {
      from = bit_scan(piece);
      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 board.pieces[White_Pieces], board.pieces[Black_Pieces]);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 1; cap_piece < 7; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << to)) {
            break;
          }
        }
        add_black_pawn_capture_move(board, from, to, cap_piece, list);
        moves &= BitBoard::clear_mask[to];
      }

      U64 enpassant = (board.enpassant == NOSQ) ? 0ULL : 1ULL << board.enpassant;
      moves = pawn_capture_moves(int_to_square[from], board.side,
                                 enpassant, board.pieces[Black_Pieces]);
      while (moves) {
        to = bit_scan(moves);
        for (cap_piece = 1; cap_piece < 7; cap_piece++) {
          if (board.pieces[cap_piece] & (1ULL << (to + 8))) {
            break;
          }
        }
        add_enpassant_move(board, MOVE(from, to, EnPassant, board.castle_perm, board.enpassant, cap_piece, None), list);
        moves &= BitBoard::clear_mask[to];
      }

      piece &= BitBoard::clear_mask[from];
    }
  }

  U64 side_pieces = (board.side == White) ? board.pieces[White_Pieces] : board.pieces[Black_Pieces];
  U64 opp_pieces = (board.side == White) ? board.pieces[Black_Pieces] : board.pieces[White_Pieces];
  int piece_type;

  // Non-Sliding Pieces
  int slide_idx = LoopNonSlideIndex[board.side];
  while (piece_type = LoopNonSlidePiece[slide_idx++]) {
    piece = board.pieces[piece_type];
    while (piece) {
      from = bit_scan(piece);
      moves = get_nonslider_moves(piece_type, int_to_square[from], side_pieces);
      while (moves) {
        to = bit_scan(moves);
        if ((1ULL << to) & opp_pieces) {
          for (cap_piece = CaptureStartIndex[board.side]; cap_piece < CaptureEndIndex[board.side]; cap_piece++) {
            if (board.pieces[cap_piece] & (1ULL << to)) {
              break;
            }
          }
          add_capture_move(board, MOVE(from, to, Normal, board.castle_perm, board.enpassant, cap_piece, None), list);
        }
        moves &= BitBoard::clear_mask[to];
      }
      piece &= BitBoard::clear_mask[from];
    }
  }

  // Sliding Pieces
  slide_idx = LoopSlideIndex[board.side];
  while (piece_type = LoopSlidePiece[slide_idx++]) {
    piece = board.pieces[piece_type];
    while (piece) {
      from = bit_scan(piece);
      moves = get_slider_moves(piece_type, int_to_square[from], board.pieces[All_Pieces], side_pieces);
      while (moves) {
        to = bit_scan(moves);
        if ((1ULL << to) & opp_pieces) {
          for (cap_piece = CaptureStartIndex[board.side]; cap_piece < CaptureEndIndex[board.side]; cap_piece++) {
            if (board.pieces[cap_piece] & (1ULL << to)) {
              break;
            }
          }
          add_capture_move(board, MOVE(from, to, Normal, board.castle_perm, board.enpassant, cap_piece, None), list);
        }
        moves &= BitBoard::clear_mask[to];
      }
      piece &= BitBoard::clear_mask[from];
    }
  }
}

void MoveGenerator::add_capture_move(const Board &board, int move, Movelist &list) {
  list.moves[list.count].move = move;

  int from = FROM_SQUARE(move);
  U64 from_bitboard = 1ULL << from;
  int piece_type;
  for (piece_type = White_Pawns; piece_type <= Black_King; piece_type++) {
    if (board.pieces[piece_type] & from_bitboard) {
      break; 
    }
  }

  assert(piece_type != 13);

  list.moves[list.count].score = mvv_lva_scores[PIECE_CAPTURED(move)][piece_type] + 1000000;
  list.count++;
}

void MoveGenerator::add_enpassant_move(const Board &board, int move, Movelist &list) {
  list.moves[list.count].move = move;
  list.moves[list.count].score = 105 + 1000000;
  list.count++;
}

void MoveGenerator::add_white_pawn_capture_move(const Board &board, const int from, const int to, 
                                                const int cap, Movelist &list) {
  if (BitBoard::rank_sq[from] == Rank_7) {
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, White_Queens), list);
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, White_Rooks), list);
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, White_Bishops), list);
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, White_Knights), list);
  }
  else {
    add_capture_move(board, MOVE(from, to, 0, board.castle_perm, board.enpassant, cap, None), list);
  }
}

void MoveGenerator::add_white_pawn_move(const Board &board, const int from, const int to, Movelist &list) {
 if (BitBoard::rank_sq[from] == Rank_7) {
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, White_Queens), list);
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, White_Rooks), list);
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, White_Bishops), list);
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, White_Knights), list);
  }
  else {
    add_quiet_move(board, MOVE(from, to, 0, board.castle_perm, board.enpassant, None, None), list);
  }
}

void MoveGenerator::add_black_pawn_capture_move(const Board &board, const int from, const int to, const int cap, Movelist &list) {
  if (BitBoard::rank_sq[from] == Rank_2) {
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, Black_Queens), list);
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, Black_Rooks), list);
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, Black_Bishops), list);
    add_capture_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, cap, Black_Knights), list);
  }
  else {
    add_capture_move(board, MOVE(from, to, 0, board.castle_perm, board.enpassant, cap, None), list);
  }
}

void MoveGenerator::add_black_pawn_move(const Board &board, const int from, const int to, Movelist &list) {
  if (BitBoard::rank_sq[from] == Rank_2) {
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, Black_Queens), list);
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, Black_Rooks), list);
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, Black_Bishops), list);
    add_quiet_move(board, MOVE(from, to, 3, board.castle_perm, board.enpassant, None, Black_Knights), list);
  }
  else {
    add_quiet_move(board, MOVE(from, to, 0, board.castle_perm, board.enpassant, None, None), list);
  }
}

static const U64 FILE_SQ[64] = {
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7,
  0, 1, 2, 3, 4, 5, 6, 7
};

static const U64 RANK_SQ[64] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7
};

U64 MoveGenerator::pawn_capture_moves(const Square &sq, const Color &color,
                                 const U64 &white_pieces, const U64 &black_pieces) {
  U64 result = 0ULL;
  U64 pos = BitBoard::set_mask[sq];
  if (color == White) {
    U64 cap_sq = black_pieces;
    if (!(FILE_SQ[sq] == File_A) && ((pos << 7) & cap_sq)) {
      result |= pos << 7;
    }
    if (!(FILE_SQ[sq] == File_H) && ((pos << 9) & cap_sq)) {
      result |= pos << 9;
    }
  }
  else {
    U64 cap_sq = white_pieces;
    if (!(FILE_SQ[sq] == File_A) && ((pos >> 9) & cap_sq)) {
      result |= pos >> 9;
    }
    if (!(FILE_SQ[sq] == File_H) && ((pos >> 7) & cap_sq)) {
      result |= pos >> 7;
    }
  }

  return result;
}

U64 MoveGenerator::pawn_quiet_moves(const Square &sq, const Color &color, const U64 &occupied) {
  U64 result = 0ULL;
  U64 pos = BitBoard::set_mask[sq];
  if (color == White) {
    if (!((pos << 8) & occupied)) {
      result |= pos << 8;
      if (RANK_SQ[sq] == Rank_2 && !((pos << 16) & occupied)) {
        result |= pos << 16;
      }
    }
  }
  else {
    if (!((pos >> 8) & occupied)) {
      result |= pos >> 8;
      if (RANK_SQ[sq] == Rank_7 && !((pos >> 16) & occupied)) {
        result |= pos >> 16;
      }
    }
  }

  return result;
}

U64 MoveGenerator::knight_moves(const Square &sq, const U64 &same_color) {
  return BitBoard::knight_moves[sq] & ~same_color;
}

U64 MoveGenerator::bishop_moves(const Square &sq, const U64 &occupied, const U64 &same_color) {
  U64 result = 0ULL;

  U64 ray = 0ULL;
  U64 intersect = 0ULL;
  U64 beyond_blocker = 0ULL;

  Direction dirs[4] = {NorthWest, NorthEast, SouthWest, SouthEast};
  bool reverse[4] = {false, false, true, true};

  for (int i = 0; i < 4; i++) {
    ray = BitBoard::ray_attacks[sq][dirs[i]];
    intersect = ray & occupied;
    if (intersect) {
      if (reverse[i]) {
        beyond_blocker = BitBoard::ray_attacks[BitBoard::bit_scan_reverse(intersect)][dirs[i]];
      }
      else {
        beyond_blocker = BitBoard::ray_attacks[BitBoard::bit_scan_forward(intersect)][dirs[i]];
      }
    }
    result |= beyond_blocker ^ ray;
    beyond_blocker = 0ULL;
  }

  return result & ~same_color;
}

U64 MoveGenerator::rook_moves(const Square &sq, const U64 &occupied, const U64 &same_color) {
  U64 result = 0ULL;

  U64 ray = 0ULL;
  U64 intersect = 0ULL;
  U64 beyond_blocker = 0ULL;

  Direction dirs[4] = {North, East, South, West};
  bool reverse[4] = {false, false, true, true};

  for (int i = 0; i < 4; i++) {
    ray = BitBoard::ray_attacks[sq][dirs[i]];
    intersect = ray & occupied;
    if (intersect) {
      if (reverse[i]) {
        beyond_blocker = BitBoard::ray_attacks[BitBoard::bit_scan_reverse(intersect)][dirs[i]];
      }
      else {
        beyond_blocker = BitBoard::ray_attacks[BitBoard::bit_scan_forward(intersect)][dirs[i]];
      }
    }
    result |= beyond_blocker ^ ray;
    beyond_blocker = 0ULL;
  }

  return result & ~same_color;
}

U64 MoveGenerator::queen_moves(const Square &sq, const U64 &occupied, const U64 &same_color) {
  return rook_moves(sq, occupied, same_color) | bishop_moves(sq, occupied, same_color);
}

U64 MoveGenerator::king_moves(const Square &sq, const U64 &same_color) {
  return BitBoard::king_moves[sq] & ~same_color;
}

U64 MoveGenerator::get_nonslider_moves(const int &piece_type, const Square &sq, const U64 &same_color) {
  switch(piece_type % 6) {
    case 2: return knight_moves(sq, same_color);
    case 0: return king_moves(sq, same_color);
    default: assert(false);
  }
}

U64 MoveGenerator::get_slider_moves(const int &piece_type, const Square &sq, const U64 &occupied, 
                                    const U64 &same_color) {
  switch (piece_type % 6) {
    case 3: return bishop_moves(sq, occupied, same_color);
    case 4: return rook_moves(sq, occupied, same_color);
    case 5: return queen_moves(sq, occupied, same_color);
    default: assert(false);
  }
}

U64 MoveGenerator::pawn_attacks(U64 pawns, Color color) {
  if (color == White) {
    return BitBoard::noEaOne(pawns) | BitBoard::noWeOne(pawns);
  }
  return BitBoard::soEaOne(pawns) | BitBoard::soWeOne(pawns);
}

U64 MoveGenerator::knight_attacks(U64 knights, U64 same_color) {
  U64 result = 0ULL;
  while (knights) {
    int sq = BitBoard::bit_scan_forward(knights);
    result |= knight_moves(int_to_square[sq], same_color);
    knights &= knights - 1;
  }
  return result;  
}

U64 MoveGenerator::bishop_attacks(U64 bishops, U64 occupied, U64 same_color) {
  U64 result = 0ULL;
  while (bishops) {
    int sq = BitBoard::bit_scan_forward(bishops);
    result |= bishop_moves(int_to_square[sq], occupied, same_color);
    bishops &= bishops - 1;
  }
  return result;  
}

U64 MoveGenerator::rook_attacks(U64 rooks, U64 occupied, U64 same_color) {
  U64 result = 0ULL;
  while (rooks) {
    int sq = BitBoard::bit_scan_forward(rooks);
    result |= rook_moves(int_to_square[sq], occupied, same_color);
    rooks &= rooks - 1;
  }
  return result; 
}

U64 MoveGenerator::queen_attacks(U64 queens, U64 occupied, U64 same_color) {
  U64 result = 0ULL;
  while (queens) {
    int sq = BitBoard::bit_scan_forward(queens);
    result |= queen_moves(int_to_square[sq], occupied, same_color);
    queens &= queens - 1;
  }
  return result; 
}

U64 MoveGenerator::king_attacks(U64 king, U64 same_color) {
  int sq = BitBoard::bit_scan_forward(king);
  return king_moves(int_to_square[sq], same_color);
}


