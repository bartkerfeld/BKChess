#pragma once

#include "board.h"
#include "search.h"

#define NAME "BKChess"
#define INPUTBUFFER 400 * 5

class Uci {
public:
  static void loop(Board& board, SearchInfo &info);
  static void parse_go(char *line, SearchInfo &info, Board &board);
  static void parse_position(char *lineIn, Board &board);
  static void read_input(SearchInfo &info);
private:
  static bool input_waiting();
};
