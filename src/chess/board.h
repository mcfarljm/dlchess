#ifndef BOARD_H_
#define BOARD_H_

#include <array>
#include <string>
#include <bitset>
#include <cstdint>
#include <optional>

#include "pieces.h"
#include "bitboard.h"
#include "hash.h"
#include "movegen.h"
#include "game_moves.h"


namespace Position = squares::Position;


namespace castling {

  enum {
    WK,
    WQ,
    BK,
    BQ,
  };

};

namespace chess {

  using squares::BOARD_SQ_NUM;

  extern const std::string_view START_FEN;

  extern const Hasher hasher;

  struct Undo {
    chess::Move mv;
    std::bitset<4> castle_perm;
    Square en_pas;
    int fifty_move;
    uint64_t hash;
  };

  class Board {
  public:

    std::array<Piece, BOARD_SQ_NUM> pieces;

    std::array<Bitboard, NUM_PIECE_TYPES_BOTH> bitboards;
    // White, black, and both-side bitboards for all pieces
    std::array<Bitboard, 3> bb_sides;

    std::array<Square, 2> king_sq = {Position::none, Position::none};

    Color side = Color::both;
    Square en_pas = Position::none;
    int fifty_move = 0;
    int total_moves = 0;

      /* ply: u32, */
      /* hist_ply: u32, */

    std::vector<Undo> history;

    std::bitset<4> castle_perm;

    uint64_t hash = 0;

    Board(const std::string_view = START_FEN);

    friend std::ostream& operator<<(std::ostream&, const Board& b);

    uint64_t get_position_hash() const;

    bool check() const;

    bool square_attacked(Square sq, Color side) const;

    chess::MoveList generate_all_moves() const;
    std::vector<chess::Move> generate_legal_moves() const;

    bool is_over() const;
    std::optional<Color> winner() const;
    int repetition_count() const;

    // makemove
    bool make_move(chess::Move mv);
    void undo_move();

    long perft(int depth);

    // io
    std::optional<chess::Move> parse_move_string(std::string_view str);

  private:
    void update_lists_and_material();
    bool is_draw_by_material() const;

    // makemove
    void hash_piece(Piece piece, Square sq);
    void hash_side();
    void hash_en_pas();
    void hash_castle();
    void clear_piece(Square sq);
    void add_piece(Piece piece, Square sq);
    void move_piece(Square from, Square to);

  };

};

#endif // BOARD_H_
