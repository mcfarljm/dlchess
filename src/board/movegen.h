#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <vector>

#include "game_moves.h"

namespace movegen {
  using game_moves::Move;
  using squares::Square;
  using pieces::Piece;

  struct MoveList {
    std::vector<Move> moves;

    // Todo: consider adding reserve(256) to constructor

    void add_white_pawn_move(Square from, Square to, Piece capture);
    void add_black_pawn_move(Square from, Square to, Piece capture);
  };
};

#endif // MOVEGEN_H
