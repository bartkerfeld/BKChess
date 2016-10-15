#include <iostream>

#include "movegen.h"
#include "zobrist.h"
#include "evaluate.h"
#include "uci.h"

int main(int argc, const char *argv[]) {
  std::cout << "BKChess Started!" << std::endl;
  BitBoard::init();
  Zobrist::init();
  MoveGenerator::init();

  Board board;
  SearchInfo info;

  Uci::loop(board, info);

  return 0;
}
