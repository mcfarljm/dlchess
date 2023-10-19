#ifndef PIECE_MOVES_H
#define PIECE_MOVES_H

#include "../bitboard.h"

using bitboard::Bitboard;

namespace piece_moves {

  Bitboard get_rook_attacks(Square sq, Bitboard occ);
  Bitboard get_bishop_attacks(Square sq, Bitboard occ);
  Bitboard get_queen_attacks(Square sq, Bitboard occ);
  
};

#endif // PIECE_MOVES_H
