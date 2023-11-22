#include "game_moves.h"

namespace game_moves {
  std::ostream& operator<<(std::ostream& os, const Move& m) {
    os << squares::square_string(m.from) << squares::square_string(m.to);

    if (m.is_promotion()) {
      auto pchar = 'q';
      if (m.promote.is_knight())
        pchar = 'n';
      else if (m.promote.is_rook())
        pchar = 'r';
      else if (m.promote.is_bishop())
        pchar = 'b';
      os << pchar;
    }

    return os;
  }
};
