#pragma once

typedef unsigned long long U64;

class Zobrist {
public:
  static void init();
  static U64 piece_keys[13][64];
  static U64 side_key;
  static U64 castle_keys[16];
};
