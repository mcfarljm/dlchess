#include "simulation.h"

#include <iostream>
#include <array>


std::pair<chess::Color, int> simulate_game(Agent* white_agent,
                                           Agent* black_agent,
                                           int verbosity,
                                           int max_moves) {
  std::array<Agent*, 2> agents {white_agent, black_agent};
  int move_count = 0;

  auto b = chess::Board();

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

  chess::Color winner;

  if (move_count >= max_moves) {
    // Assign a draw if move count exceeded.
    winner = chess::Color::both;
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
