#include <iostream>
#include <cassert>

#include "bitboard.h"

U64 BitBoard::set_mask[64] = {0};
U64 BitBoard::clear_mask[64] = {0};
U64 BitBoard::ray_attacks[64][8] = {0};
U64 BitBoard::knight_moves[64] = {0};
U64 BitBoard::king_moves[64] = {0};
U64 BitBoard::rank[8] = {0};
U64 BitBoard::file[8] = {0};
U64 BitBoard::rank_sq[64] = {0};

// Table used for bit_scan
static const int index64[64] = {
   0, 47,  1, 56, 48, 27,  2, 60,
  57, 49, 41, 37, 28, 16,  3, 61,
  54, 58, 35, 52, 50, 42, 21, 44,
  38, 32, 29, 23, 17, 11,  4, 62,
  46, 55, 26, 59, 40, 36, 15, 53,
  34, 51, 20, 43, 31, 22, 10, 45,
  25, 39, 14, 33, 19, 30,  9, 24,
  13, 18,  8, 12,  7,  6,  5, 63
};

static U64 not_file_a, not_file_ab, not_file_h, not_file_gh, not_rank_1, not_rank_8;

static U64 noNoEa(const U64 &bb) {return (bb & not_file_h) << 17;}
static U64 noEaEa(const U64 &bb) {return (bb & not_file_gh) << 10;}
static U64 soEaEa(const U64 &bb) {return (bb & not_file_gh) >>  6;}
static U64 soSoEa(const U64 &bb) {return (bb & not_file_h) >> 15;}
static U64 noNoWe(const U64 &bb) {return (bb & not_file_a) << 15;}
static U64 noWeWe(const U64 &bb) {return (bb & not_file_ab) <<  6;}
static U64 soWeWe(const U64 &bb) {return (bb & not_file_ab) >> 10;}
static U64 soSoWe(const U64 &bb) {return (bb & not_file_a) >> 17;}
static U64 nortOne(const U64 &bb) {return (bb & not_rank_8) << 8;}
static U64 soutOne(const U64 &bb) {return (bb & not_rank_1) >> 8;}
static U64 eastOne(const U64 &bb) {return (bb & not_file_h) << 1;}
static U64 westOne(const U64 &bb) {return (bb & not_file_a) >> 1;}
U64 BitBoard::noEaOne(const U64 &bb) {return (bb & not_file_h) << 9;}
U64 BitBoard::soEaOne(const U64 &bb) {return (bb & not_file_h) >> 7;}
U64 BitBoard::soWeOne(const U64 &bb) {return (bb & not_file_a) >> 9;}
U64 BitBoard::noWeOne(const U64 &bb) {return (bb & not_file_a) << 7;}

void BitBoard::init() {
  U64 rank_1 = 0xFFULL;
  U64 file_a = 0x0101010101010101;

  for (int i = 0; i < 8; i++) {
    rank[i] = rank_1 << (8 * i);
    file[i] = file_a << i;
  }

  not_file_a = ~file[File_A];
  not_file_ab = not_file_a & ~file[File_B];
  not_file_h = ~file[File_H];
  not_file_gh = ~file[File_G] & not_file_h;
  not_rank_1 = ~rank[Rank_1];
  not_rank_8 = ~rank[Rank_8];

  U64 king_sq, knight_sq, attacks;
  for (int sq = 0; sq < 64; sq++) {
    king_sq = 1ULL << sq;
    attacks = eastOne(king_sq) | westOne(king_sq);
    king_sq |= attacks;
    attacks |= nortOne(king_sq) | soutOne(king_sq);
    king_moves[sq] = attacks;

    knight_sq = 1ULL << sq;
    knight_moves[sq] = noNoEa(knight_sq) | noEaEa(knight_sq) | soEaEa(knight_sq) | 
                       soSoEa(knight_sq) | noNoWe(knight_sq) | noWeWe(knight_sq) | 
                       soWeWe(knight_sq) | soSoWe(knight_sq);

    set_mask[sq] = 1ULL << sq;
    clear_mask[sq] = ~set_mask[sq];
    rank_sq[sq] = sq / 8;
  }

  // ray_attacks
  U64 nort = 0x0101010101010100ULL;
  for (int sq = 0; sq < 64; sq++, nort <<= 1) {
    ray_attacks[sq][North] = nort;
  }
  U64 sout = 0x0080808080808080ULL;
  for (int sq = 63; sq >= 0; sq--, sout >>= 1) {
    ray_attacks[sq][South] = sout;
  }
  U64 east = 0xFEULL;
  for (int f = 0; f < 8; f++, east = eastOne(east)) {
    U64 ea = east;
    for (int r8 = 0; r8 < 8*8; r8 += 8, ea <<= 8) {
      ray_attacks[r8 + f][East] = ea;
    }
  }
  U64 west = 0x7F;
  for (int f = 7; f >= 0; f--, west = westOne(west)) {
    U64 we = west;
    for (int r8 = 0; r8 < 8*8; r8 += 8, we <<= 8) {
      ray_attacks[r8 + f][West] = we;
    }
  }
  U64 noea = 0x8040201008040200ULL;
  for (int f = 0; f < 8; f++, noea = eastOne(noea)) {
    U64 ne = noea;
    for (int r8 = 0; r8 < 8*8; r8 += 8, ne <<= 8) {
      ray_attacks[r8 + f][NorthEast] = ne;
    }
  }
  U64 nowe = 0x0102040810204000ULL;
  for (int f = 7; f >= 0; f--, nowe = westOne(nowe)) {
    U64 nw = nowe;
    for (int r8 = 0; r8 < 8*8; r8 += 8, nw <<= 8) {
      ray_attacks[r8 + f][NorthWest] = nw;
    }
  }
  U64 soea = 0x0002040810204080ULL;
  for (int f = 0; f < 8; f++, soea = eastOne(soea)) {
    U64 se = soea;
    for (int r8 = 56; r8 >= 0; r8 -= 8, se >>= 8) {
      ray_attacks[r8 + f][SouthEast] = se;
    }
  }
  U64 sowe = 0x0040201008040201ULL;
  for (int f = 7; f >= 0; f--, sowe = westOne(sowe)) {
    U64 sw = sowe;
    for (int r8 = 56; r8 >= 0; r8 -= 8, sw >>= 8) {
      ray_attacks[r8 + f][SouthWest] = sw;
    }
  }
}

int BitBoard::bit_scan_forward(U64 bb) {
  assert(bb);
  U64 debruijn64 = 0x03f79d71b4cb0a89ULL;
  return index64[((bb ^ (bb-1)) * debruijn64) >> 58];
}

int BitBoard::bit_scan_reverse(U64 bb) {
  assert(bb);
  U64 debruijn64 = 0x03f79d71b4cb0a89ULL;
  bb |= bb >> 1;
  bb |= bb >> 2;
  bb |= bb >> 4;
  bb |= bb >> 8;
  bb |= bb >> 16;
  bb |= bb >> 32;
  return index64[(bb * debruijn64) >> 58];
}

int BitBoard::count_bits(U64 bb) {
  int r;
  for (r = 0; bb; r++, bb &= bb - 1);
  return r;
}

void BitBoard::print_bitboard(const U64 &bb) {
  int rank[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  for (int i = 56; i >= 0; i -= 8) {
    std::cout << rank[i / 8] << " ";
    for (int j = 0; j < 8; j++) {
      if ( (1ULL << (i + j)) & bb ) {
        std::cout << " X ";
      }
      else {
        std::cout << " - ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << "   " << "A  B  C  D  E  F  G  H" << std::endl;
  return;
}
