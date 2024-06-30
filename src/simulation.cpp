#include "simulation.h"

#include <iostream>


std::pair<Color, int> simulate_game(Agent* white_agent,
                                    Agent* black_agent,
                                    int verbosity,
                                    int max_moves) {
  Agent* agents[2] = {white_agent, black_agent};
  int move_count = 0;

  auto b = board::Board();

  while (move_count < max_moves && ! b.is_over()) {
    if (verbosity >= 3)
      std::cout << b;
    auto move = agents[static_cast<int>(b.side)]->select_move(b);
    if (verbosity >= 2)
      std::cout << "Move " << b.total_moves + 1 << ": " << move << std::endl;
    if (verbosity >= 3)
      std::cout << std::endl;
    b.make_move(move);
    ++move_count;
  }

  Color winner;

  if (move_count >= max_moves) {
    // Assign a draw if move count exceeded.
    winner = Color::both;
  }
  else {
    winner = b.winner().value(); // NOLINT
  }

  if (verbosity >= 1) {
    std::cout << move_count << " moves\n";
    std::cout << "Winner: " << static_cast<int>(winner) << std::endl;
  }

  return std::make_pair(winner, move_count);
}
