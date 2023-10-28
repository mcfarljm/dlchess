#include <iostream>

#include "bitboard.h"


namespace bitboard {

  std::ostream& operator<<(std::ostream& os, const Bitboard& b) {
    for (auto rank : squares::RANKS_REVERSED) {
      for (auto file : squares::FILES) {
        auto sq = squares::fr_to_sq(file, rank);
        os << (b[sq] ? "x" : "-");
      }
      os << std::endl;
    }
    return os;
  }

};
