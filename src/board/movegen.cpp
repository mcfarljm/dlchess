#include <array>

#include "movegen.h"

using game_moves::MoveFlag;

namespace movegen {
  constexpr Piece white_promotion_pieces[] = {Piece::WN, Piece::WB, Piece::WR, Piece::WQ};
  constexpr Piece black_promotion_pieces[] = {Piece::BN, Piece::BB, Piece::BR, Piece::BQ};

  void MoveList::add_white_pawn_move(Square from, Square to, Piece capture) {
    if (from / 8 == squares::RANK_7) {
      // Add a version of the move with each possible promotion
      for (auto promote : white_promotion_pieces) {
        moves.emplace_back(from, to, capture, promote, MoveFlag::none);
      }
    }
    else
      moves.emplace_back(from, to, capture);
  }

  void MoveList::add_black_pawn_move(Square from, Square to, Piece capture) {
    if (from / 8 == squares::RANK_2) {
      // Add a version of the move with each possible promotion
      for (auto promote : black_promotion_pieces) {
        moves.emplace_back(from, to, capture, promote, MoveFlag::none);
      }
    }
    else
      moves.emplace_back(from, to, capture);
  }
};
