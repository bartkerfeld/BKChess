#pragma once

#include "board.h"

class MoveMaker {
public:
  static void take_move(Board &board);
  static bool make_move(Board &board, const int move);
  static bool move_exists(Board &board, const int move);
private:
  static void clear_piece(const int sq, Board &board);
  static void add_piece(const int sq, Board &board, const int piece);
  static void move_piece(const int from, const int to, Board &board);
};
