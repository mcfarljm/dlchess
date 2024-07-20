#ifndef GAME_MOVES_H
#define GAME_MOVES_H

#include <iostream>

#include "../squares.h"
#include "../pieces.h"

namespace chess {

  using squares::Square;

  enum class MoveFlag {
    none,
    enpas,
    pawnstart,
    castle,
  };

  struct Move {
    Square from;
    Square to;
    Piece capture;
    Piece promote;
    MoveFlag flag;

    Move(Square from, Square to) :
      from(from), to(to), capture(Piece::none),
      promote(Piece::none), flag(MoveFlag::none) {}

    Move(Square from, Square to, Piece capture) :
      from(from), to(to), capture(capture),
      promote(Piece::none), flag(MoveFlag::none) {}

    Move(Square from, Square to, Piece capture, Piece promote, MoveFlag flag) :
      from(from), to(to), capture(capture),
      promote(promote), flag(flag) {}

    bool is_capture() const {
      return capture.exists();
    }

    bool is_promotion() const {
      return promote.exists();
    }

    bool is_underpromotion() const {
      return promote.exists() && ! promote.is_queen();
    }

    bool is_en_pas() const {
      return flag == MoveFlag::enpas;
    }

    bool is_castle() const {
      return flag == MoveFlag::castle;
    }

    bool is_pawn_start() const {
      return flag == MoveFlag::pawnstart;
    }

    bool operator==(const Move& rhs) const {
      return (from == rhs.from) && (to == rhs.to) && (promote == rhs.promote);
    }

    friend std::ostream& operator<<(std::ostream& os, const Move& m);

  };

  class MoveHash {
  public:
    std::size_t operator() (const Move& m) const {
      return std::hash<Square>()(m.from) ^ std::hash<Square>()(m.to) ^ std::hash<int>()(m.promote.value);
    }
  };

};


#endif // GAME_MOVES_H
