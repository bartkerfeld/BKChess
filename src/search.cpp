#include <iostream>
#include <cassert>

#include "search.h"
#include "makemove.h"
#include "pvtable.h"
#include "evaluate.h"
#include "time.h"
#include "uci.h"

#define INFINITE 30000
#define MATE 29000

void Searcher::search_position(Board &board, SearchInfo &info) {
  int best_move = NOMOVE;
  int best_score = -INFINITE;
  int current_depth = 0;
  int pv_moves = 0;
  int pv_num = 0;

  clear_for_search(board, info);

  for (current_depth = 1; current_depth <= info.depth; current_depth++) {
    best_score = alpha_beta(-INFINITE, INFINITE, current_depth, board, info, true);

    if (info.stopped) {
      break;
    }

    pv_moves = PvTable::get_pv_line(current_depth, board);
    best_move = board.pv_array[0];

    std::cout << "info score cp " << best_score << " depth " << current_depth << " nodes "
              << info.nodes << " time " << Time::get_current_time() - info.start_time << " ";

    pv_moves = PvTable::get_pv_line(4, board);
    std::cout << "pv";
    for (int pv_num = 0; pv_num < pv_moves; pv_num++) {
      std::cout << " " << MoveGenerator::get_move(board.pv_array[pv_num]);
    }
    std::cout << std::endl;
  }

  std::cout << "bestmove " << MoveGenerator::get_move(best_move) << std::endl;
}

int Searcher::alpha_beta(int alpha, int beta, int depth, Board &board, SearchInfo &info, bool do_null) {
  if (depth == 0) {
    info.nodes++;
    return quiescence(alpha, beta, board, info);
  }

  if ((info.nodes & 2047) == 0) {
    check_up(info);
  }

  info.nodes++;

  if ((is_repetition(board) || board.fifty_move >= 100) && board.ply) {
    return 0;
  }

  if (board.ply > MAXDEPTH - 1) {
    return Evaluator::evaluate_positon(board);
  }

  U64 king_bitboard;
  Color attacker_color;

  if (board.side == White) {
    king_bitboard = board.pieces[White_King];
    attacker_color = Black;
  }
  else {
    king_bitboard = board.pieces[Black_King];
    attacker_color = White;
  }
  int king_square = BitBoard::bit_scan_forward(king_bitboard);
  if (MoveGenerator::square_attacked(int_to_square[king_square], attacker_color, board)) {
    depth++;
  }

  Movelist list;
  MoveGenerator::generate_moves(board, list);

  int move_num = 0;
  int legal = 0;
  int old_alpha = alpha;
  int best_move = NOMOVE;
  int score = -INFINITE;
  int pv_move = PvTable::probe_table(board);

  if (pv_move != NOMOVE) {
    for (move_num = 0; move_num < list.count; move_num++) {
      if (list.moves[move_num].move == pv_move) {
        list.moves[move_num].score = 2000000;
      }
    }
  }

  for (move_num = 0; move_num < list.count; move_num++) {
    pick_next_move(move_num, list);

    if (!MoveMaker::make_move(board, list.moves[move_num].move)) {
      continue;
    }

    legal++;
    score = -alpha_beta(-beta, -alpha, depth - 1, board, info, true);
    MoveMaker::take_move(board);

    if (info.stopped) {
      return 0;
    }

    if (score > alpha) {
      if (score >= beta) {
        if (legal == 1) {
          info.fail_high_first++;
        }
        info.fail_high++;

        if (!(list.moves[move_num].move & (0xF << 24))) {
          board.search_killers[1][board.ply] = board.search_killers[0][board.ply]; 
          board.search_killers[0][board.ply] = list.moves[move_num].move;
        }

        return beta;
      }
      alpha = score;
      best_move = list.moves[move_num].move;

      if (!(list.moves[move_num].move & (0xF << 24))) {
        int from = FROM_SQUARE(list.moves[move_num].move);
        U64 from_bitboard = 1ULL << from;
        int piece_type;
        for (piece_type = White_Pawns; piece_type <= Black_King; piece_type++) {
          if (board.pieces[piece_type] & from_bitboard) {
            break; 
          }
        }
        assert(piece_type != 13);

        board.search_history[piece_type][TO_SQUARE(best_move)] += depth;
      }
    }
  }

  if (legal == 0) {
    Color attacker_side;
    U64 king_bitboard;

    if (board.side == White) {
      attacker_side = Black;
      king_bitboard = board.pieces[White_King];
    }
    else {
      attacker_side = White;
      king_bitboard = board.pieces[Black_King];
    }
    
    int king_square = BitBoard::bit_scan_forward(king_bitboard);
    if (MoveGenerator::square_attacked(int_to_square[king_square], attacker_side, board)) {
      return -MATE + board.ply;
    }
    else {
      return 0;
    }
  }

  if (alpha != old_alpha) {
    PvTable::store_move(board, best_move);
  }

  return alpha;
}

int Searcher::quiescence(int alpha, int beta, Board &board, SearchInfo &info) {
  if ((info.nodes & 2047) == 0) {
    check_up(info);
  }

  info.nodes++;

  if (is_repetition(board) || board.fifty_move >= 100) {
    return 0;
  }

  if (board.ply > MAXDEPTH - 1) {
    return Evaluator::evaluate_positon(board);
  }

  int score = Evaluator::evaluate_positon(board);

  if (score >= beta) {
    return beta;
  }

  if (score > alpha) {
    alpha = score;
  }

  Movelist list;
  MoveGenerator::generate_capture_moves(board, list);

  int move_num = 0;
  int legal = 0;
  int old_alpha = alpha;
  int best_move = NOMOVE;
  score = -INFINITE;
  int pv_move = PvTable::probe_table(board);

  for (move_num = 0; move_num < list.count; move_num++) {
    pick_next_move(move_num, list);

    if (!MoveMaker::make_move(board, list.moves[move_num].move)) {
      continue;
    }

    legal++;
    score = -quiescence(-beta, -alpha, board, info);
    MoveMaker::take_move(board);

    if (info.stopped) {
      return 0;
    }

    if (score > alpha) {
      if (score >= beta) {
        if (legal == 1) {
          info.fail_high_first++;
        }
        info.fail_high++;
        return beta;
      }
      alpha = score;
      best_move = list.moves[move_num].move;
    }
  }
  
  if (alpha != old_alpha) {
    PvTable::store_move(board, best_move);
  }

  return alpha;
}

void Searcher::pick_next_move(int move_num, Movelist &list) {
  Move temp;

  int best_score = 0;
  int best_index = move_num;
  
  for (int i = move_num; i < list.count; i++) {
    if (list.moves[i].score > best_score) {
      best_score = list.moves[i].score;
      best_index = i;
    }
  }

  temp = list.moves[move_num];
  list.moves[move_num] = list.moves[best_index];
  list.moves[best_index] = temp;
}

void Searcher::check_up(SearchInfo &info) {
  if (info.time_set == true && Time::get_current_time() > info.stop_time) {
    info.stopped = true;
  }

  Uci::read_input(info);
}

void Searcher::clear_for_search(Board &board, SearchInfo &info) {
  for (int i = 0; i < 13; i++) {
    for (int j = 0; j < 64; j++) {
      board.search_history[i][j] = 0;
    }
  }

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < MAXDEPTH; j++) {
      board.search_killers[i][j] = 0;
    }
  }

  PvTable::clear();
  board.ply = 0;

  info.stopped = 0;
  info.nodes = 0;
  info.fail_high = 0;
  info.fail_high_first = 0;
}

bool Searcher::is_repetition(const Board &board) {
  for (int i = board.history_ply - board.fifty_move; i < board.history_ply - 1; i++) {
    assert(i >= 0 && i < 2048);

    if (board.position_key == board.history[i].position_key) {
      return true;
    }
  }
  return false;
}
