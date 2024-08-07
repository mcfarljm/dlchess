#include "agent_random.h"
#include "myrand.h"


using chess::Move;
using chess::Board;

Move RandomAgent::select_move(const Board& b) {
  auto candidates = b.generate_legal_moves();

  if (candidates.empty())
    throw std::runtime_error("No legal moves");

  std::uniform_int_distribution<> dist(0, candidates.size() - 1);  // NOLINT(bugprone-narrowing-conversions)
  auto idx = dist(rng);
  return candidates[idx];
}
