#include <stdexcept>
#include <cassert>
#include <iostream>

#include "board.h"
#include "hash.h"


using pieces::Piece;
using pieces::Color;

namespace board {

  // Initialize random hash keys:
  const Hasher hasher {};

  Board Board::from_fen(const std::string_view fen) {
    Board board;

    auto rank = squares::RANK_8;
    auto file = squares::FILE_A;
    Piece piece;
    int count;
    char c;

    auto it = fen.begin();

    // This is essentially just a while(true) loop that must be
    // broken out off
    while (rank >= squares::RANK_1) {
      c = *it++;
      count = 1;
      switch (c) {
      case 'p': piece = Piece::BP; break;
      case 'r': piece = Piece::BR; break;
      case 'n': piece = Piece::BN; break;
      case 'b': piece = Piece::BB; break;
      case 'k': piece = Piece::BK; break;
      case 'q': piece = Piece::BQ; break;

      case 'P': piece = Piece::WP; break;
      case 'R': piece = Piece::WR; break;
      case 'N': piece = Piece::WN; break;
      case 'B': piece = Piece::WB; break;
      case 'K': piece = Piece::WK; break;
      case 'Q': piece = Piece::WQ; break;

      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
        piece = Piece::none;
        count = c - '0';
        break;

      case '/': case ' ':
        file = squares::FILE_A;
        if (rank <= squares::RANK_1)
          goto exit_loop;
        rank--;
        continue;

      default:
        throw std::runtime_error("FEN error");
      } // End switch

      if (piece.exists()) {
        for (int i=0; i<count; ++i) {
          auto sq = squares::fr_to_sq(file, rank);
          board.pieces[sq] = piece;
          ++file;
        }
      }
    }
  exit_loop:

    c = *it++;
    switch (c) {
    case 'w':
      board.side = Color::white;
      break;
    case 'b':
      board.side = Color::black;
      break;
    default:
      throw std::runtime_error("unexpected FEN side color character");
    }

    // Castling permissions:
    ++it;
    c = *it++;
    for (int i=0; i<4; ++i) {
      switch (c) {
      case 'K':
        board.castle_perm |= castling::WK; break;
      case 'Q':
        board.castle_perm |= castling::WQ; break;
      case 'k':
        board.castle_perm |= castling::BK; break;
      case 'q':
        board.castle_perm |= castling::BQ; break;
      case '-':
        break;
      case ' ':
        goto exit_castling_loop;
      default:
        throw std::runtime_error("unexpected FEN castling permission character");
      }
      c = *it++;
    }
  exit_castling_loop:

    // En passant
    c = *it++;

    if (c != '-') {
      file = c - 'a';
      c = *it++;
      rank = c - '1';
      assert(file >= squares::FILE_A && file <= squares::FILE_H);
      assert(rank >= squares::RANK_1 && rank <= squares::RANK_8);
      board.en_pas = squares::fr_to_sq(file, rank);
    }

    board.hash = board.get_position_hash();
    board.update_lists_and_material();

    return board;
  }

  void Board::update_lists_and_material() {
    for (Square sq=0; sq<64; ++sq) {
      auto piece = pieces[sq];

      if (piece.exists()) {
        auto color = piece.color();

        if (piece.is_king())
          king_sq[static_cast<int>(color)] = sq;
        bitboards[piece.value].set_bit(sq);
        bb_sides[static_cast<int>(color)].set_bit(sq);
        bb_sides[static_cast<int>(Color::both)].set_bit(sq);
      }
    }
  }

  std::ostream& operator<<(std::ostream& os, const Board& b) {

    static constexpr std::string_view side_chars = "wb-";
    static constexpr std::string_view file_chars = "abcdefgh";

    for (auto rank : squares::RANKS_REVERSED) {
      os << rank + 1 << "     ";
      for (auto file : squares::FILES) {
        auto sq = squares::fr_to_sq(file, rank);
        auto piece = b.pieces[sq];
        os << piece << "  ";
      }
      os << std::endl;
    }
    os << std::endl << "      ";

    for (auto file : squares::FILES)
      os << file_chars[file] << "  ";

    os << std::endl;
    os << "side: " << side_chars[static_cast<int>(b.side)] << std::endl;
    os << "enPas: " << b.en_pas << std::endl;

    os << "castle: ";
    os << ((b.castle_perm & castling::WK).any() ? 'K' : '-');
    os << ((b.castle_perm & castling::WQ).any() ? 'Q' : '-');
    os << ((b.castle_perm & castling::BK).any() ? 'k' : '-');
    os << ((b.castle_perm & castling::BQ).any() ? 'q' : '-');
    os << std::endl;

    return os;
  }

  uint64_t Board::get_position_hash() const {
    uint64_t hash = 0;

    for (Square sq=0; sq<BOARD_SQ_NUM; ++sq) {
      auto piece = pieces[sq];
      if (piece.exists())
        hash ^= hasher.piece_keys[static_cast<int>(piece.value)][sq];
    }

    if (side == Color::white)
      hash ^= hasher.side_key;

    if (en_pas != static_cast<Square>(Position::none))
      hash ^= hasher.piece_keys[static_cast<int>(Piece::none)][en_pas];

    hash ^= hasher.castle_keys[castle_perm.to_ulong()];

    return hash;
  }

  bool Board::check() const {
    std::array<int, pieces::NUM_PIECE_TYPES_BOTH> piece_count{};

    auto board_assert = [](bool condition) {
      if (!condition)
        throw std::runtime_error("board check failure");
    };

    for (Square sq=0; sq<BOARD_SQ_NUM; ++sq) {
      auto piece = pieces[sq];
      if (piece.exists())
        ++piece_count[static_cast<int>(piece.value)];
    }

    for (auto piece_index=0; piece_index<pieces::NUM_PIECE_TYPES_BOTH; ++piece_index)
      board_assert(piece_count[piece_index] == bitboards[piece_index].count());

    // Check pawn bitboard squares:
    for (auto sq : bitboards[static_cast<int>(pieces::Piece::WP)])
      board_assert(pieces[sq] == pieces::Piece::WP);
    for (auto sq : bitboards[static_cast<int>(pieces::Piece::BP)])
      board_assert(pieces[sq] == pieces::Piece::BP);

    board_assert(side == Color::white || side == Color::black);
    board_assert(hash == get_position_hash());

    board_assert(en_pas == static_cast<Square>(Position::none) ||
                 (en_pas/8 == squares::RANK_6 && side == Color::white) ||
                 (en_pas/8 == squares::RANK_3 && side == Color::black));

    board_assert(pieces[king_sq[static_cast<int>(Color::white)]] == pieces::Piece::WK);
    board_assert(pieces[king_sq[static_cast<int>(Color::black)]] == pieces::Piece::BK);
    board_assert(bitboards[static_cast<int>(pieces::Piece::WK)].bits[king_sq[static_cast<int>(Color::white)]]);
    board_assert(bitboards[static_cast<int>(pieces::Piece::BK)].bits[king_sq[static_cast<int>(Color::black)]]);

    // Check side piece bitboards:

    //     assert_eq!(self.bb_sides[WHITE].count(), PIECE_TYPES.iter().filter(|p| p.color()==WHITE).map(|&p| self.bitboards[p as usize].count()).sum());
    //     assert_eq!(self.bb_sides[BLACK].count(), PIECE_TYPES.iter().filter(|p| p.color()==BLACK).map(|&p| self.bitboards[p as usize].count()).sum());

    return true;

  }

};
