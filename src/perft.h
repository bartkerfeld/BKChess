#pragma once

#include "board.h"

class Perft {
public:
  static int test(int depth, Board &board);
  static int test_no_print(int depth, Board &board);
private:
  static void perft(int depth, Board &board);
  static long leaf_nodes;
};
