#include <iostream>
#include "movegen.h"
#include "makemove.h"
#include "perft.h"
#include "time.h"

long Perft::leaf_nodes = 0;

int Perft::test(int depth, Board &board) {
  std::cout << std::endl << "Starting Test to Depth: " << depth << std::endl;

  leaf_nodes = 0;

  int start_time = Time::get_current_time();

  Movelist list;
  MoveGenerator::generate_moves(board, list);

  int move; 
  int move_num = 0;
  for (move_num = 0; move_num < list.count; ++move_num) {
    move = list.moves[move_num].move;
    if (!MoveMaker::make_move(board, move)) {
      continue;
    }
    long cumnodes = leaf_nodes;
    perft(depth - 1, board);
    MoveMaker::take_move(board);
    long oldnodes = leaf_nodes - cumnodes;
    std::cout << "move " << move_num+1 << " " << MoveGenerator::get_move(move) << " " << oldnodes << std::endl;
  }

  std::cout << std::endl << "Test Complete : " << leaf_nodes << " nodes visited in " 
            << Time::get_current_time() - start_time << "ms" << std::endl;
  return leaf_nodes;
}

int Perft::test_no_print(int depth, Board &board) {
  //std::cout << std::endl << "Starting Test to Depth: " << depth << std::endl;

  leaf_nodes = 0;

  Movelist list;
  MoveGenerator::generate_moves(board, list);

  int move; 
  int move_num = 0;
  for (move_num = 0; move_num < list.count; ++move_num) {
    move = list.moves[move_num].move;
    if (!MoveMaker::make_move(board, move)) {
      continue;
    }
    long cumnodes = leaf_nodes;
    perft(depth - 1, board);
    MoveMaker::take_move(board);
    long oldnodes = leaf_nodes - cumnodes;
    //std::cout << "move " << move_num+1 << " " << MoveGenerator::get_move(move) << " " << oldnodes << std::endl;
  }

  //std::cout << std::endl << "Test Complete : " << leaf_nodes << " nodes visited" << std::endl;
  return leaf_nodes;
}

void Perft::perft(int depth, Board &board) {
  if (depth == 0) {
    leaf_nodes++;
    return;
  }

  Movelist list;
  MoveGenerator::generate_moves(board, list);

  int move_num = 0;
  int move = 0;

  for (move_num = 0; move_num < list.count; ++move_num) {
    move = list.moves[move_num].move;

    //board.print_board();
    if (!MoveMaker::make_move(board, move)) {
      continue;
    }
    perft(depth - 1, board);
    MoveMaker::take_move(board);
  }

}


