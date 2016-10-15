#pragma once

#include "board.h"

class Evaluator {
public:
  static int evaluate_positon(Board board);
private:
  static int get_material_score(const Board &board);
};
