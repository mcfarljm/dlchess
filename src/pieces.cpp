#include "pieces.h"

namespace pieces {

  Color swap_side(Color side) {
    switch(side) {
    case Color::white:
      return Color::black;
    case Color::black:
      return Color::white;
    default:
      return Color::both;
    }
  }

  std::ostream& operator<<(std::ostream& os, const Piece& piece) {
    switch (piece.value) {
    case Piece::WP: os << "P"; break;
    case Piece::WN: os << "N"; break;
    case Piece::WB: os << "B"; break;
    case Piece::WR: os << "R"; break;
    case Piece::WQ: os << "Q"; break;
    case Piece::WK: os << "K"; break;
    case Piece::BP: os << "p"; break;
    case Piece::BN: os << "n"; break;
    case Piece::BB: os << "b"; break;
    case Piece::BR: os << "r"; break;
    case Piece::BQ: os << "q"; break;
    case Piece::BK: os << "k"; break;
    case Piece::none: os << "."; break;
    }
    return os;
  }

};
