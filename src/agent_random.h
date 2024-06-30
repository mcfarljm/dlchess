#ifndef AGENT_RANDOM_H
#define AGENT_RANDOM_H

#include "agent_base.h"


class RandomAgent : public Agent {
public:
  game_moves::Move select_move(const board::Board& b) override;
};


#endif // AGENT_RANDOM_H
