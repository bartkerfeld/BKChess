#pragma once

#include "movegen.h"
#include "board.h"

typedef struct {
  int start_time;
  int stop_time;
  int depth;
  int depth_set;
  int time_set;
  int moves_to_go;
  int infinite;

  long nodes;
  int quit;
  int stopped;

  float fail_high;
  float fail_high_first;
} SearchInfo;

class Searcher {
public:
  static void search_position(Board &board, SearchInfo &info);
private:
  static void check_up(SearchInfo &info);
  static void clear_for_search(Board &board, SearchInfo &info);
  static int alpha_beta(int alpha, int beta, int depth, Board &board, SearchInfo &info, bool do_null);
  static int quiescence(int alpha, int beta, Board &board, SearchInfo &info);
  static void pick_next_move(int move_num, Movelist &list);
  static bool is_repetition(const Board &board);
};
