#pragma once

#include "board.h"

typedef unsigned long long U64;

typedef struct {
  U64 position_key;
  int move;
} PvEntry;

class PvTable {
public:
  static int get_pv_line(const int depth, Board &board);
  static int probe_table(const Board &board);
  static void store_move(const Board &board, const int move);
  static void init();
  static void clear();
  static void free_table();
private:
  static PvEntry *table;
  static int num_entries;
};
