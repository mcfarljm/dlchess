#include "game_moves.h"

namespace game_moves {
  std::ostream& operator<<(std::ostream& os, const Move& m) {
    os << squares::square_string(m.from) << squares::square_string(m.to);

    if (m.is_promotion()) {
      auto pchar = 'q';
      if (m.promote.is_knight())
        pchar = 'n';
      else if (m.promote.is_rook_or_queen() && ! m.promote.is_bishop_or_queen())
        pchar = 'r';
      else if (m.promote.is_bishop_or_queen() && ! m.promote.is_rook_or_queen())
        pchar = 'b';
      os << pchar;
    }

    return os;
  }
};
