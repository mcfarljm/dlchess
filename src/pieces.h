#ifndef PIECES_H
#define PIECES_H

#include <iostream>

namespace pieces {

  enum class Color {
    white,
    black,
    both,
  };

  constexpr int NUM_PIECE_TYPES_BOTH = 12;

  /// Wrap piece enum in a class to provide methods and encapsulation.
  ///
  /// Another idea for handling none would be to remove the none enum value, and
  /// then use optional<Piece> when storing a piece.  This improves type safety,
  /// as methods that expect a piece don't need to check for none.  But it may
  /// increase memory due to the optional storing a bool and an enum.
  class Piece {
  public:
    enum Value {
      WP, WN, WB, WR, WQ, WK,
      BP, BN, BB, BR, BQ, BK,
      none,
    };

    Piece() = default;
    constexpr Piece(Value v) : value(v) {}

    constexpr bool operator==(Piece a) const { return value == a.value; }
    constexpr bool operator!=(Piece a) const { return value != a.value; }

    constexpr bool exists() const { return value != none; }

    Color color() const {
      switch(value) {
      case WP: case WN: case WB: case WR: case WQ: case WK:
        return Color::white;
      case BP: case BN: case BB: case BR: case BQ: case BK:
        return Color::black;
      default:
        return Color::both;
      }
    }

    constexpr bool is_king() const {
      return (value == WK || value == BK);
    }

    friend std::ostream& operator<<(std::ostream& os, const Piece& piece);
      
  // private:
    Value value = none;
  };

  // SLIDERS[color] produces an array that can be iterated through
  constexpr Piece SLIDERS[2][3] = {
    {Piece::WB, Piece::WR, Piece::WQ},
    {Piece::BB, Piece::BR, Piece::BQ},
  };
  constexpr Piece NON_SLIDERS[2][2] = {
    {Piece::WN, Piece::WK},
    {Piece::BN, Piece::BK},
  };

};

#endif // PIECES_H
