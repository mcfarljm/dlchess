#ifndef AGENT_BASE_H
#define AGENT_BASE_H

#include "board/board.h"

class Agent {
 public:
  virtual game_moves::Move select_move(const board::Board&) = 0;
};

#endif // AGENT_BASE_H
