#ifndef AGENT_BASE_H
#define AGENT_BASE_H

#include "board/board.h"

class Agent {
 public:
  virtual chess::Move select_move(const chess::Board&) = 0;
  virtual void set_search_time(std::optional<int> move_time_ms,
                               std::optional<int> time_left_ms,
                               std::optional<int> inc_ms,
                               const chess::Board& b) {}
};

#endif // AGENT_BASE_H
