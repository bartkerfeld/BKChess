#include <cstdlib>

#include "zobrist.h"

#define RAND_64 (  (U64)rand() | \
                   (U64)rand() << 15 | \
                   (U64)rand() << 30 | \
                   (U64)rand() << 45 | \
                   ((U64)rand() & 0xF) << 60  )
                  

U64 Zobrist::piece_keys[13][64] = {0};
U64 Zobrist::side_key = 0;
U64 Zobrist::castle_keys[16] = {0};

void Zobrist::init() {
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 64; j++) {
      piece_keys[i][j] = RAND_64;
    }
  }

  side_key = RAND_64;

  for (int i = 0; i < 16; i++) {
    castle_keys[i] = RAND_64;
  }
}
