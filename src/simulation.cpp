#include "simulation.h"

#include <iostream>


std::pair<Color, int> simulate_game(Agent* black_agent,
                                    Agent* white_agent,
                                    int verbosity) {
  Agent* agents[2] = {black_agent, white_agent};
  int move_count = 0;

  auto b = board::Board::from_fen(board::START_FEN);

  while (! b.is_over()) {
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

  auto winner = b.winner().value();
  if (verbosity >= 1) {
    std::cout << move_count << " moves\n";
    std::cout << "Winner: " << static_cast<int>(winner) << std::endl;
  }
  return std::make_pair(winner, move_count);
}
