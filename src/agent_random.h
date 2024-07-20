#ifndef AGENT_RANDOM_H
#define AGENT_RANDOM_H

#include "agent_base.h"


class RandomAgent : public Agent {
public:
  chess::Move select_move(const chess::Board& b) override;
};


#endif // AGENT_RANDOM_H
