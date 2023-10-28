#ifndef BOARD_H_
#define BOARD_H_

#include <array>
#include <string>
#include <bitset>
#include <cstdint>

#include "../pieces.h"
#include "../bitboard.h"
#include "movegen.h"


using pieces::Piece;
using pieces::Color;
using bitboard::Bitboard;
namespace Position = squares::Position;


namespace castling {

  enum {
    WK,
    WQ,
    BK,
    BQ,
  };

};

namespace board {

  constexpr int BOARD_SQ_NUM = 64;

  extern const std::string_view START_FEN;

  class Board {
  public:

    std::array<Piece, BOARD_SQ_NUM> pieces;

    std::array<Bitboard, pieces::NUM_PIECE_TYPES_BOTH> bitboards;
    // White, black, and both-side bitboards for all pieces
    std::array<Bitboard, 3> bb_sides;

    std::array<Square, 2> king_sq = {Position::none, Position::none};

    Color side = Color::both;
    Square en_pas = Position::none;
    int fifty_move = 0;

      /* ply: u32, */
      /* hist_ply: u32, */

      /* pub history: Vec<Undo>, */

    std::bitset<4> castle_perm;

    uint64_t hash = 0;

      /* hash_keys: HashKeys, */

    static Board from_fen(const std::string_view);

    friend std::ostream& operator<<(std::ostream&, const Board& b);

    uint64_t get_position_hash() const;

    bool check() const;

    bool square_attacked(Square sq, Color side) const;

    movegen::MoveList generate_all_moves() const;

  private:
    void update_lists_and_material();
  };


};

#endif // BOARD_H_
