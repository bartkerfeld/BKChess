#include <iostream>
#include <cassert>

#include "board.h"
#include "zobrist.h"
#include "pvtable.h"

void Board::init() {
  reset();
  pieces[White_Pawns] = 0xFFULL << 8;
  pieces[White_Knights] = 1ULL << B1 | 1ULL << G1;
  pieces[White_Bishops] = 1ULL << C1 | 1ULL << F1;
  pieces[White_Rooks] = 1ULL << A1 | 1ULL << H1;
  pieces[White_Queens] = 1ULL << D1;
  pieces[White_King] = 1ULL << E1;
  pieces[Black_Pawns] = 0xFFULL << 48;
  pieces[Black_Knights] = 1ULL << B8 | 1ULL << G8;
  pieces[Black_Bishops] = 1ULL << C8 | 1ULL << F8;
  pieces[Black_Rooks] = 1ULL << A8 | 1ULL << H8;
  pieces[Black_Queens] = 1ULL << D8;
  pieces[Black_King] = 1ULL << E8;

  pieces[White_Pieces] = pieces[White_Pawns] | pieces[White_Knights] | pieces[White_Bishops] |
                 pieces[White_Rooks] | pieces[White_Queens] | pieces[White_King];
  pieces[Black_Pieces] = pieces[Black_Pawns] | pieces[Black_Knights] | pieces[Black_Bishops] |
                 pieces[Black_Rooks] | pieces[Black_Queens] | pieces[Black_King];
  pieces[All_Pieces] = pieces[White_Pieces] | pieces[Black_Pieces];

  generate_position_key(*this);
  enpassant = NOSQ;
  castle_perm = 0xF;
  history_ply = 0;
  ply = 0;
  side = White;
}

void Board::reset() {
  for (int i = 0; i < 16; i++) {
    pieces[i] = 0ULL;
  }
  fifty_move = 0;
  history_ply = 0;
  ply = 0;
  castle_perm = 0;
  enpassant = NOSQ;
  position_key = 0ULL;
}

const Color piece_color[13] = { 
  Black,
  White, White, White, White, White, White,
  Black, Black, Black, Black, Black, Black
};

static const U64 file_rank_to_sq[8][8] = {
  {1ULL << A1, 1ULL << A2, 1ULL << A3, 1ULL << A4, 1ULL << A5, 1ULL << A6, 1ULL << A7, 1ULL << A8},
  {1ULL << B1, 1ULL << B2, 1ULL << B3, 1ULL << B4, 1ULL << B5, 1ULL << B6, 1ULL << B7, 1ULL << B8},
  {1ULL << C1, 1ULL << C2, 1ULL << C3, 1ULL << C4, 1ULL << C5, 1ULL << C6, 1ULL << C7, 1ULL << C8},
  {1ULL << D1, 1ULL << D2, 1ULL << D3, 1ULL << D4, 1ULL << D5, 1ULL << D6, 1ULL << D7, 1ULL << D8},
  {1ULL << E1, 1ULL << E2, 1ULL << E3, 1ULL << E4, 1ULL << E5, 1ULL << E6, 1ULL << E7, 1ULL << E8},
  {1ULL << F1, 1ULL << F2, 1ULL << F3, 1ULL << F4, 1ULL << F5, 1ULL << F6, 1ULL << F7, 1ULL << F8},
  {1ULL << G1, 1ULL << G2, 1ULL << G3, 1ULL << G4, 1ULL << G5, 1ULL << G6, 1ULL << G7, 1ULL << G8},
  {1ULL << H1, 1ULL << H2, 1ULL << H3, 1ULL << H4, 1ULL << H5, 1ULL << H6, 1ULL << H7, 1ULL << H8}
};

int Board::parse_fen(char *fen) {
  assert(fen != NULL);

  int rank = 7; // RANK 8
  int file = 0; // FILE A
  int piece = 0;
  int count = 0;
  int i = 0;
  int sq64 = 0;

  reset();

  while ((rank >= 0) && *fen) {
    count = 1;
    switch (*fen) {
      case 'p': piece = Black_Pawns; break;
      case 'r': piece = Black_Rooks; break;
      case 'n': piece = Black_Knights; break;
      case 'b': piece = Black_Bishops; break;
      case 'k': piece = Black_King; break;
      case 'q': piece = Black_Queens; break;
      case 'P': piece = White_Pawns; break;
      case 'R': piece = White_Rooks; break;
      case 'N': piece = White_Knights; break;
      case 'B': piece = White_Bishops; break;
      case 'K': piece = White_King; break;
      case 'Q': piece = White_Queens; break;
      
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        piece = None;
        count = *fen - '0';
        break;

      case '/':
      case ' ':
        rank--;
        file = 0;
        fen++;
        continue;

      default:
        std::cout << "FEN Error!" << std::endl;
        return -1;
    }

    for (i = 0; i < count; i++) {
      sq64 = rank * 8 + file;
      U64 bb = 1ULL << sq64;
      if (piece != None) {
        pieces[piece] |= bb;
        pieces[All_Pieces] |= bb;
        if (piece_color[piece] == White) {
          pieces[White_Pieces] |= bb;
        }
        else {
          pieces[Black_Pieces] |= bb; 
        }
      }
      file++;
    }
    fen++;
  }

  assert(*fen == 'w' || *fen == 'b');

  side = (*fen == 'w') ? White : Black;
  fen += 2;

  for (i = 0; i < 4; i++) {
    if (*fen == ' ' ) {
      break;
    }
    switch(*fen) {
      case 'K': castle_perm |= WKCA; break;
      case 'Q': castle_perm |= WQCA; break;
      case 'k': castle_perm |= BKCA; break;
      case 'q': castle_perm |= BQCA; break;
      default: break;
    }
    fen++;
  }
  fen++;

  assert(castle_perm >= 0 && castle_perm <= 15);

  if (*fen != '-') {
    file = fen[0] - 'a';
    rank = fen[1] - '1';

    assert(file >= 0 && file <= 7);
    assert(rank >= 0 && rank <= 7);

    enpassant = BitBoard::bit_scan_forward(file_rank_to_sq[file][rank]);
    std::cout << "enpassant is " << enpassant << std::endl;
  }

  generate_position_key(*this);

  return 0;
}

void Board::generate_position_key(Board board) {
  position_key = 0ULL;

  for (int i = 1; i < 13; i++) {
    while (board.pieces[i]) {
      if (board.pieces[i] != 0ULL) {
        int sq = BitBoard::bit_scan_forward(board.pieces[i]);
        position_key ^= Zobrist::piece_keys[i][sq];
        board.pieces[i] &= BitBoard::clear_mask[sq];
      }
    }
  }

  if (side == White) {
    position_key ^= Zobrist::side_key;
  }

  if (enpassant == NOSQ) {
    position_key ^= Zobrist::piece_keys[None][enpassant];
  }

  position_key ^= Zobrist::castle_keys[castle_perm];
}

void Board::print_board() {
  int rank[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  for (int i = 56; i >= 0; i -= 8) {
    std::cout << rank[i / 8] << " ";
    for (int j = 0; j < 8; j++) {
      U64 pos = 1ULL << (i + j);
      if (pos & pieces[All_Pieces]) {
        std::string piece = " - ";

        if (pos & pieces[White_Pawns]) {
          piece = " P ";
        }
        else if (pos & pieces[White_Knights]) {
          piece = " N ";
        }
        else if (pos & pieces[White_Bishops]) {
          piece = " B ";
        }
        else if (pos & pieces[White_Rooks]) {
          piece = " R ";
        }
        else if (pos & pieces[White_Queens]) {
          piece = " Q ";
        }
        else if (pos & pieces[White_King]) {
          piece = " K ";
        }
        else if (pos & pieces[Black_Pawns]) {
          piece = " p ";
        }
        else if (pos & pieces[Black_Knights]) {
          piece = " n ";
        }
        else if (pos & pieces[Black_Bishops]) {
          piece = " b ";
        }
        else if (pos & pieces[Black_Rooks]) {
          piece = " r ";
        }
        else if (pos & pieces[Black_Queens]) {
          piece = " q ";
        }
        else if (pos & pieces[Black_King]) {
          piece = " k ";
        }

        std::cout << piece;
      }
      else {
        std::cout << " - ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  std::cout << "   " << "A  B  C  D  E  F  G  H" << std::endl;
  std::cout << "side: " << (side == White ? "w" : "b") << std::endl;
  std::cout << "enpassant: " << enpassant << std::endl;
  std::cout << "castle: " << castle_perm << std::endl;
  std::cout << "key: " << position_key << std::endl;
}
