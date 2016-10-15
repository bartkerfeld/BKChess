#pragma once

typedef unsigned long long U64;

enum Square {
  A1, B1, C1, D1, E1, F1, G1, H1,
  A2, B2, C2, D2, E2, F2, G2, H2,
  A3, B3, C3, D3, E3, F3, G3, H3,
  A4, B4, C4, D4, E4, F4, G4, H4,
  A5, B5, C5, D5, E5, F5, G5, H5,
  A6, B6, C6, D6, E6, F6, G6, H6,
  A7, B7, C7, D7, E7, F7, G7, H7,
  A8, B8, C8, D8, E8, F8, G8, H8, NOSQ
};

enum Direction {
  North, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest
};

enum Color {
  White, Black
};

enum Rank {
  Rank_1, Rank_2, Rank_3, Rank_4, Rank_5, Rank_6, Rank_7, Rank_8
};

enum File {
  File_A, File_B, File_C, File_D, File_E, File_F, File_G, File_H
};

class BitBoard {
public:
  static void init();
  static int bit_scan_forward(U64 bb);
  static int bit_scan_reverse(U64 bb);
  static int count_bits(U64 bb);
  static void print_bitboard(const U64 &bb);

  static U64 noEaOne(const U64 &bb);
  static U64 noWeOne(const U64 &bb);
  static U64 soEaOne(const U64 &bb);
  static U64 soWeOne(const U64 &bb);

  static U64 set_mask[64];
  static U64 clear_mask[64];
  static U64 ray_attacks[64][8];
  static U64 knight_moves[64];
  static U64 king_moves[64];
  static U64 rank[8];
  static U64 file[8];
  static U64 rank_sq[64];
};
