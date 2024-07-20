#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <vector>

#include "game_moves.h"

namespace chess {
  using squares::Square;

  struct MoveList {
    std::vector<Move> moves;

    void add_white_pawn_move(Square from, Square to, Piece capture);
    void add_black_pawn_move(Square from, Square to, Piece capture);
  };
};

#endif // MOVEGEN_H
