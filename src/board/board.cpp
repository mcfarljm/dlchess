#include <stdexcept>
#include <cassert>
#include <iostream>

#include "board.h"
#include "piece_moves.h"


using pieces::Piece;
using pieces::Color;
using piece_moves::white_pawn_attacks;
using piece_moves::black_pawn_attacks;
using piece_moves::knight_moves;
using piece_moves::king_moves;

namespace board {

  const std::string_view START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  // Initialize random hash keys:
  const Hasher hasher {};

  Board::Board(const std::string_view fen) {
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

      for (int i=0; i<count; ++i) {
        if (piece.exists()) {
          auto sq = squares::fr_to_sq(file, rank);
          pieces[sq] = piece;
        }
        ++file;
      }
    }
  exit_loop:

    c = *it++;
    switch (c) {
    case 'w':
      side = Color::white;
      break;
    case 'b':
      side = Color::black;
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
        castle_perm.set(castling::WK); break;
      case 'Q':
        castle_perm.set(castling::WQ); break;
      case 'k':
        castle_perm.set(castling::BK); break;
      case 'q':
        castle_perm.set(castling::BQ); break;
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
      en_pas = squares::fr_to_sq(file, rank);
    }

    hash = get_position_hash();
    update_lists_and_material();
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
    os << "Fifty: " << b.fifty_move << std::endl;

    os << "castle: ";
    os << (b.castle_perm[castling::WK] ? 'K' : '-');
    os << (b.castle_perm[castling::WQ] ? 'Q' : '-');
    os << (b.castle_perm[castling::BK] ? 'k' : '-');
    os << (b.castle_perm[castling::BQ] ? 'q' : '-');
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

    if (en_pas != Position::none)
      hash ^= hasher.piece_keys[static_cast<int>(Piece::none)][en_pas];

    hash ^= hasher.castle_keys[castle_perm.to_ulong()];

    return hash;
  }

  bool Board::check() const {
    std::array<int, pieces::NUM_PIECE_TYPES_BOTH> piece_count{};

    auto board_assert = [](bool condition, std::string_view msg) {
      if (!condition) {
        // std::cout << *this;
        std::string full_msg = "board check failure: ";
        full_msg += msg;
        throw std::runtime_error(full_msg);
      }
    };

    for (Square sq=0; sq<BOARD_SQ_NUM; ++sq) {
      auto piece = pieces[sq];
      if (piece.exists())
        ++piece_count[static_cast<int>(piece.value)];
    }

    for (auto piece_index=0; piece_index<pieces::NUM_PIECE_TYPES_BOTH; ++piece_index)
      board_assert(piece_count[piece_index] == bitboards[piece_index].count(),
                   "piece count matches bitboard count");

    // Check pawn bitboard squares:
    for (auto sq : bitboards[static_cast<int>(pieces::Piece::WP)])
      board_assert(pieces[sq] == pieces::Piece::WP, "WP bitboard");
    for (auto sq : bitboards[static_cast<int>(pieces::Piece::BP)])
      board_assert(pieces[sq] == pieces::Piece::BP, "BP bitboard");

    board_assert(side == Color::white || side == Color::black, "side");
    board_assert(hash == get_position_hash(), "hash");

    board_assert(en_pas == Position::none ||
                 (en_pas/8 == squares::RANK_6 && side == Color::white) ||
                 (en_pas/8 == squares::RANK_3 && side == Color::black),
                 "en_pas");

    board_assert(pieces[king_sq[static_cast<int>(Color::white)]] == pieces::Piece::WK, "WK");
    board_assert(pieces[king_sq[static_cast<int>(Color::black)]] == pieces::Piece::BK, "BK");
    board_assert(bitboards[static_cast<int>(pieces::Piece::WK)][king_sq[static_cast<int>(Color::white)]], "WK BB");
    board_assert(bitboards[static_cast<int>(pieces::Piece::BK)][king_sq[static_cast<int>(Color::black)]], "BK BB");

    // Check side piece bitboards:

    //     assert_eq!(self.bb_sides[WHITE].count(), PIECE_TYPES.iter().filter(|p| p.color()==WHITE).map(|&p| self.bitboards[p as usize].count()).sum());
    //     assert_eq!(self.bb_sides[BLACK].count(), PIECE_TYPES.iter().filter(|p| p.color()==BLACK).map(|&p| self.bitboards[p as usize].count()).sum());

    return true;

  }

  bool Board::square_attacked(Square sq, Color side) const {
    assert(check());

    // Pawns
    if (side == Color::white) {
      if ((bitboards[static_cast<int>(pieces::Piece::WP)] & black_pawn_attacks[sq]).any())
        return true;
    }
    else {
      if ((bitboards[static_cast<int>(pieces::Piece::BP)] & white_pawn_attacks[sq]).any())
        return true;
    }

    // Knights
    auto knight_piece = (side == Color::white ? Piece::WN : Piece::BN);
    if ((knight_moves[sq] & bitboards[static_cast<int>(knight_piece)]).any())
      return true;

    auto occ = bb_sides[static_cast<int>(Color::both)];

    // Bishops or queens
    auto bishops_queens = (side == Color::white ?
                           bitboards[static_cast<int>(Piece::WB)] | bitboards[static_cast<int>(Piece::WQ)] :
                           bitboards[static_cast<int>(Piece::BB)] | bitboards[static_cast<int>(Piece::BQ)]);
    if ((piece_moves::get_bishop_attacks(sq, occ) & bishops_queens).any())
      return true;

    // Rooks or queens
    auto rooks_queens = (side == Color::white ?
                           bitboards[static_cast<int>(Piece::WR)] | bitboards[static_cast<int>(Piece::WQ)] :
                           bitboards[static_cast<int>(Piece::BR)] | bitboards[static_cast<int>(Piece::BQ)]);
    if ((piece_moves::get_rook_attacks(sq, occ) & rooks_queens).any())
      return true;

    // Kings
    auto king_piece = (side == Color::white ? Piece::WK : Piece::BK);
    if ((king_moves[sq] & bitboards[static_cast<int>(king_piece)]).any())
      return true;

    return false;
  }

  int Board::repetition_count() const {
    int repetitions = 0;
    for (auto& item : history) {
      if (item.hash == hash)
        ++repetitions;
    }
    return repetitions;
  }

  bool Board::is_draw_by_material() const {
    if (bitboards[static_cast<int>(Piece::WP)].any() ||
        bitboards[static_cast<int>(Piece::BP)].any())
      return false;
    if (bitboards[static_cast<int>(Piece::WQ)].any() ||
        bitboards[static_cast<int>(Piece::BQ)].any() ||
        bitboards[static_cast<int>(Piece::WR)].any() ||
        bitboards[static_cast<int>(Piece::BR)].any())
      return false;
    if (bitboards[static_cast<int>(Piece::WB)].count() > 1 ||
        bitboards[static_cast<int>(Piece::BB)].count() > 1)
      return false;
    if (bitboards[static_cast<int>(Piece::WN)].count() > 1 ||
        bitboards[static_cast<int>(Piece::BN)].count() > 1)
      return false;
    if (bitboards[static_cast<int>(Piece::WN)].any() &&
        bitboards[static_cast<int>(Piece::WB)].any())
      return false;
    if (bitboards[static_cast<int>(Piece::BN)].any() &&
        bitboards[static_cast<int>(Piece::BB)].any())
      return false;

    // Otherwise, it must be a draw:
    return true;
  }

  bool Board::is_over() const {
    // This move count may not be exact (?)
    if (fifty_move > 100)
      return true;
    if (repetition_count() >= 2)
      return true;
    if (is_draw_by_material())
      return true;

    // Check for legal move:
    auto board_copy = *this;
    bool found = false;
    auto move_list = generate_all_moves();
    for (auto& mv : move_list.moves) {
      if (! board_copy.make_move(mv))
        continue;
      found = true;
      board_copy.undo_move();
      break;
    }
    return ! found;
  }

  std::optional<Color> Board::winner() const {
    // This move count may not be exact (?)
    if (fifty_move > 100)
      return Color::both;
    if (repetition_count() >= 2)
      return Color::both;
    if (is_draw_by_material())
      return Color::both;

    // Check for legal move:
    auto board_copy = *this;
    bool found = false;
    auto move_list = generate_all_moves();
    for (auto& mv : move_list.moves) {
      if (! board_copy.make_move(mv))
        continue;
      found = true;
      board_copy.undo_move();
      break;
    }
    if (found)
      return std::nullopt;

    const bool in_check = square_attacked(king_sq[static_cast<int>(side)], other_side(side));
    if (in_check) {
      return other_side(side);
    }
    else
      // Stalemate
      return Color::both;
  }

};
