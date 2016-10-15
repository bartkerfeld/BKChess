#include <iostream>
#include <cassert>
#include <stdlib.h>
#include "pvtable.h"
#include "movegen.h"
#include "makemove.h"

PvEntry *PvTable::table = 0;
int PvTable::num_entries = 0;

static const int SIZE = 0x100000 * 2;

int PvTable::get_pv_line(const int depth, Board &board) {
  assert(depth < MAXDEPTH);

  int move = probe_table(board);
  int count = 0;

  while (move != NOMOVE && count < depth) {
    assert(count < MAXDEPTH);

    if (MoveMaker::move_exists(board, move)) {
      MoveMaker::make_move(board, move);
      board.pv_array[count++] = move;
    }
    else {
      break;
    }
    move = probe_table(board);
  }

  while (board.ply > 0) {
    MoveMaker::take_move(board);
  }

  return count;
}

int PvTable::probe_table(const Board &board) {
  int index = board.position_key % num_entries;

  assert(index >= 0 && index <= num_entries - 1);

  if (table[index].position_key == board.position_key) {
    return table[index].move;
  }

  return NOMOVE;
}

void PvTable::store_move(const Board &board, const int move) {
  int index = board.position_key % num_entries;

  assert(index >= 0 && index <= num_entries - 1);

  table[index].move = move;
  table[index].position_key = board.position_key;
}

void PvTable::init() {
  num_entries = SIZE / sizeof(PvEntry);
  num_entries -= 2;
  
  table = (PvEntry*) malloc(num_entries * sizeof(PvEntry));
  clear();

  std::cout << "PvTable init complete with " << num_entries << " entries" << std::endl;
}

void PvTable::clear() {
  for (PvEntry *entry = table; entry < table + num_entries; entry++) {
    entry->position_key = 0ULL;
    entry->move = 0;
  }
}

void PvTable::free_table() {
  free(table);
}
