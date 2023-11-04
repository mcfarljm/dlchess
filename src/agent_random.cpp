#include "agent_random.h"
#include "myrand.h"


using game_moves::Move;
using board::Board;

Move RandomAgent::select_move(const Board& b) {
  auto move_list = b.generate_all_moves();
 
  auto board_copy = b;

  std::vector<Move> candidates;

  for (auto& mv : move_list.moves) {
    if (! board_copy.make_move(mv))
      continue;
    candidates.push_back(mv);
    board_copy.undo_move();
  }

  if (candidates.empty())
    throw std::runtime_error("No legal moves");

  std::uniform_int_distribution<> dist(0, candidates.size() - 1);
  auto idx = dist(rng);
  return candidates[idx];
}
