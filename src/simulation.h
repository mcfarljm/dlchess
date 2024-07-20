#ifndef SIMULATION_H
#define SIMULATION_H

#include <utility>

#include "agent_base.h"


/// Simulate game and return (winner, num_moves)
std::pair<chess::Color, int> simulate_game(Agent* black_agent,
                                    Agent* white_agent,
                                    int verbosity = 0,
                                    int max_moves = 50000);


#endif // SIMULATION_H
