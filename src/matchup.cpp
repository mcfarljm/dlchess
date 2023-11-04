#include <iostream>
#include <memory>

#include "simulation.h"
#include "agent_random.h"

int main(int argc, const char* argv[]) {

  int verbosity = 3;

  auto black_agent = std::make_unique<RandomAgent>();
  auto white_agent = std::make_unique<RandomAgent>();

  auto [winner, num_moves] = simulate_game(black_agent.get(), white_agent.get(), verbosity);


  return 0;
}
