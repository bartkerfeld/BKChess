#include "evaluate.h"
#include "bitboard.h"

static const int PawnTable[64] = {
  0, 0, 0, 0, 0, 0, 0, 0, 
  10, 10, 0, -10, -10, 0, 10, 10, 
  5, 0, 0, 5, 5, 0, 0, 5, 
  0, 0, 10, 20, 20, 10, 0, 0, 
  5, 5, 5, 10, 10, 5, 5, 5, 
  10, 10, 10, 20, 20, 10, 10, 10, 
  20, 20, 20, 30, 30, 20, 20, 20, 
  0, 0, 0, 0, 0, 0, 0, 0
};

static const int KnightTable[64] = {
  0, -10, 0, 0, 0, 0, -10, 0, 
  0, 0, 0, 5, 5, 0, 0, 0, 
  0, 0, 10, 10, 10, 10, 0, 0, 
  0, 0, 10, 20, 20, 10, 5, 0, 
  5, 10, 15, 20, 20, 15, 10, 5, 
  5, 10, 10, 20, 20, 10, 10, 5, 
  0, 0, 5, 10, 10, 5, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0
};

static const int BishopTable[64] = {
  0, 0, -10, 0, 0, -10, 0, 0, 
  0, 0, 0, 10, 10, 0, 0, 0, 
  0, 0, 10, 15, 15, 10, 0, 0, 
  0, 10, 15, 20, 20, 15, 10, 0, 
  0, 10, 15, 20, 20, 15, 10, 0, 
  0, 0, 10, 15, 15, 10, 0, 0, 
  0, 0, 0, 10, 10, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0
};

static const int RookTable[64] = {
  0, 0, 5, 10, 10, 5, 0, 0, 
  0, 0, 5, 10, 10, 5, 0, 0, 
  0, 0, 5, 10, 10, 5, 0, 0, 
  0, 0, 5, 10, 10, 5, 0, 0, 
  0, 0, 5, 10, 10, 5, 0, 0, 
  0, 0, 5, 10, 10, 5, 0, 0, 
  25, 25, 25, 25, 25, 25, 25, 25, 
  0, 0, 5, 10, 10, 5, 0, 0
};

static const int Mirror64[64] = {
  56, 57, 58, 59, 60, 61, 62, 63, 
  48, 49, 50, 51, 52, 53, 54, 55, 
  40, 41, 42, 43, 44, 45, 46, 47, 
  32, 33, 34, 35, 36, 37, 38, 39, 
  24, 25, 26, 27, 28, 29, 30, 31, 
  16, 17, 18, 19, 20, 21, 22, 23, 
  8, 9, 10, 11, 12, 13, 14, 15, 
  0, 1, 2, 3, 4, 5, 6, 7
};

static const int PieceVal[13] = 
  { 0, 100, 325, 325, 550, 1000, 50000, 100, 325, 325, 550, 1000, 50000 };

int Evaluator::evaluate_positon(Board board) {
  int score = get_material_score(board);

  while (board.pieces[White_Pawns]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[White_Pawns]);
    score += PawnTable[sq];
    board.pieces[White_Pawns] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[White_Knights]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[White_Knights]);
    score += KnightTable[sq];
    board.pieces[White_Knights] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[White_Bishops]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[White_Bishops]);
    score += BishopTable[sq];
    board.pieces[White_Bishops] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[White_Rooks]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[White_Rooks]);
    score += RookTable[sq];
    board.pieces[White_Rooks] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[Black_Pawns]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[Black_Pawns]);
    score -= PawnTable[Mirror64[sq]];
    board.pieces[Black_Pawns] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[Black_Knights]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[Black_Knights]);
    score -= KnightTable[Mirror64[sq]];
    board.pieces[Black_Knights] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[Black_Bishops]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[Black_Bishops]);
    score -= BishopTable[Mirror64[sq]];
    board.pieces[Black_Bishops] &= BitBoard::clear_mask[sq];
  }

  while (board.pieces[Black_Rooks]) {
    int sq = BitBoard::bit_scan_forward(board.pieces[Black_Rooks]);
    score -= RookTable[Mirror64[sq]];
    board.pieces[Black_Rooks] &= BitBoard::clear_mask[sq];
  }

  if (board.side == White) {
    return score;
  }
  return -score;
}

int Evaluator::get_material_score(const Board &board) {
  int white_material = 0;
  int black_material = 0;
  for (int i = 1; i < 7; i++) {
    int num_white_piece = BitBoard::count_bits(board.pieces[i]);
    int num_black_piece = BitBoard::count_bits(board.pieces[i + 6]);

    white_material += num_white_piece * PieceVal[i];
    black_material += num_black_piece * PieceVal[i + 6]; 
  }

  return white_material - black_material;
}
