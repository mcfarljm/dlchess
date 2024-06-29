#include "agent_random.h"
#include "myrand.h"


using game_moves::Move;
using board::Board;

Move RandomAgent::select_move(const Board& b) {
  auto candidates = b.generate_legal_moves();

  if (candidates.empty())
    throw std::runtime_error("No legal moves");

  std::uniform_int_distribution<> dist(0ul, candidates.size() - 1ul);
  auto idx = dist(rng);
  return candidates[idx];
}
