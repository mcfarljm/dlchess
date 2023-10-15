#ifndef BOARD_H_
#define BOARD_H_

#include <array>
#include <string>
#include <bitset>
#include <cstdint>

#include "../pieces.h"
#include "../bitboard.h"


using pieces::Piece;
using pieces::Color;
using bitboard::Bitboard;
using squares::Position;


namespace castling {

  constexpr std::bitset<4> WK(1);
  constexpr std::bitset<4> WQ(2);
  constexpr std::bitset<4> BK(4);
  constexpr std::bitset<4> BQ(8);

};

namespace board {

  constexpr int BOARD_SQ_NUM = 64;

  static constexpr std::string_view START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  class Board {
  public:

    std::array<Piece, BOARD_SQ_NUM> pieces;

    std::array<Bitboard, pieces::NUM_PIECE_TYPES_BOTH> bitboards;
    // White, black, and both-side bitboards for all pieces
    std::array<Bitboard, 3> bb_sides;

    std::array<Square, 2> king_sq = {static_cast<Square>(Position::none),
                                     static_cast<Square>(Position::none)};

    Color side = Color::both;
    Square en_pas = static_cast<Square>(Position::none);
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

  private:
    void update_lists_and_material();
  };


};

#endif // BOARD_H_
