#include <array>

#include "movegen.h"
#include "board.h"
#include "piece_moves.h"

using chess::MoveFlag;

namespace chess {
  constexpr Piece white_promotion_pieces[] = {Piece::WN, Piece::WB, Piece::WR, Piece::WQ};
  constexpr Piece black_promotion_pieces[] = {Piece::BN, Piece::BB, Piece::BR, Piece::BQ};

  void MoveList::add_white_pawn_move(Square from, Square to, Piece capture) {
    if (from / 8 == squares::RANK_7) {
      // Add a version of the move with each possible promotion
      for (auto promote : white_promotion_pieces) {
        moves.emplace_back(from, to, capture, promote, MoveFlag::none);
      }
    }
    else
      moves.emplace_back(from, to, capture);
  }

  void MoveList::add_black_pawn_move(Square from, Square to, Piece capture) {
    if (from / 8 == squares::RANK_2) {
      // Add a version of the move with each possible promotion
      for (auto promote : black_promotion_pieces) {
        moves.emplace_back(from, to, capture, promote, MoveFlag::none);
      }
    }
    else
      moves.emplace_back(from, to, capture);
  }
};

namespace chess {
  MoveList Board::generate_all_moves() const {
    MoveList move_list;
    move_list.moves.reserve(128);

    if (side == Color::white) {
      // Pawn non-captures:
      auto to_step1 = (bitboards[static_cast<int>(Piece::WP)] << 8) & (~ bb_sides[static_cast<int>(Color::both)]);
      auto to_step2 = (to_step1 << 8) & BB_RANK_4 & (~ bb_sides[static_cast<int>(Color::both)]);

      for (auto to64 : Bitboard(to_step1))
        move_list.add_white_pawn_move(to64 - 8, to64, Piece::none);
      for (auto to64 : Bitboard(to_step2))
        move_list.moves.emplace_back(to64 - 2*8, to64, Piece::none, Piece::none, MoveFlag::pawnstart);

      // Pawn captures:
      auto to_cap_left = ((bitboards[static_cast<int>(Piece::WP)] & ~ BB_FILE_A) << 7) & bb_sides[static_cast<int>(Color::black)];
      auto to_cap_right = ((bitboards[static_cast<int>(Piece::WP)] & ~ BB_FILE_H) << 9) & bb_sides[static_cast<int>(Color::black)];
      for (auto to64 : Bitboard(to_cap_left))
        move_list.add_white_pawn_move(to64 - 7, to64, pieces[to64]);
      for (auto to64 : Bitboard(to_cap_right))
        move_list.add_white_pawn_move(to64 - 9, to64, pieces[to64]);

      // En passant captures:
      if (en_pas != Position::none) {
        auto ep_bb = Bitboard(1ULL << en_pas);
        auto ep_to_left = ((bitboards[static_cast<int>(Piece::WP)] & ~ BB_FILE_A) << 7) & ep_bb;
        auto ep_to_right = ((bitboards[static_cast<int>(Piece::WP)] & ~ BB_FILE_H) << 9) & ep_bb;

        for (auto to64 : Bitboard(ep_to_left))
          move_list.moves.emplace_back(to64 - 7, to64, Piece::none, Piece::none, MoveFlag::enpas);
        for (auto to64 : Bitboard(ep_to_right))
          move_list.moves.emplace_back(to64 - 9, to64, Piece::none, Piece::none, MoveFlag::enpas);
      }

      // Castling
      if (castle_perm[castling::WK]) {
        if (pieces[Position::F1] == Piece::none && pieces[Position::G1] == Piece::none) {
          if ((! square_attacked(Position::E1, Color::black)) &&
              (! square_attacked(Position::F1, Color::black)))
            move_list.moves.emplace_back(Position::E1, Position::G1, Piece::none, Piece::none, MoveFlag::castle);
        }
      }
      if (castle_perm[castling::WQ]) {
        if (pieces[Position::D1] == Piece::none && pieces[Position::C1] == Piece::none && pieces[Position::B1] == Piece::none) {
          if ((! square_attacked(Position::E1, Color::black)) &&
              (! square_attacked(Position::D1, Color::black)))
            move_list.moves.emplace_back(Position::E1, Position::C1, Piece::none, Piece::none, MoveFlag::castle);
        }
      }
    }
    else { // black

      // Pawn non-captures:
      auto to_step1 = (bitboards[static_cast<int>(Piece::BP)] >> 8) & (~ bb_sides[static_cast<int>(Color::both)]);
      auto to_step2 = (to_step1 >> 8) & BB_RANK_5 & (~ bb_sides[static_cast<int>(Color::both)]);

      for (auto to64 : Bitboard(to_step1))
        move_list.add_black_pawn_move(to64 + 8, to64, Piece::none);
      for (auto to64 : Bitboard(to_step2))
        move_list.moves.emplace_back(to64 + 2*8, to64, Piece::none, Piece::none, MoveFlag::pawnstart);

      // Pawn captures:
      auto to_cap_left = ((bitboards[static_cast<int>(Piece::BP)] & ~ BB_FILE_A) >> 9) & bb_sides[static_cast<int>(Color::white)];
      auto to_cap_right = ((bitboards[static_cast<int>(Piece::BP)] & ~ BB_FILE_H) >> 7) & bb_sides[static_cast<int>(Color::white)];
      for (auto to64 : Bitboard(to_cap_left))
        move_list.add_black_pawn_move(to64 + 9, to64, pieces[to64]);
      for (auto to64 : Bitboard(to_cap_right))
        move_list.add_black_pawn_move(to64 + 7, to64, pieces[to64]);

      // En passant captures:
      if (en_pas != Position::none) {
        auto ep_bb = Bitboard(1ULL << en_pas);
        auto ep_to_left = ((bitboards[static_cast<int>(Piece::BP)] & ~ BB_FILE_A) >> 9) & ep_bb;
        auto ep_to_right = ((bitboards[static_cast<int>(Piece::BP)] & ~ BB_FILE_H) >> 7) & ep_bb;

        for (auto to64 : Bitboard(ep_to_left))
          move_list.moves.emplace_back(to64 + 9, to64, Piece::none, Piece::none, MoveFlag::enpas);
        for (auto to64 : Bitboard(ep_to_right))
          move_list.moves.emplace_back(to64 + 7, to64, Piece::none, Piece::none, MoveFlag::enpas);
      }

      // Castling
      if (castle_perm[castling::BK]) {
        if (pieces[Position::F8] == Piece::none && pieces[Position::G8] == Piece::none) {
          if ((! square_attacked(Position::E8, Color::white)) &&
              (! square_attacked(Position::F8, Color::white)))
            move_list.moves.emplace_back(Position::E8, Position::G8, Piece::none, Piece::none, MoveFlag::castle);
        }
      }
      if (castle_perm[castling::BQ]) {
        if (pieces[Position::D8] == Piece::none && pieces[Position::C8] == Piece::none && pieces[Position::B8] == Piece::none) {
          if ((! square_attacked(Position::E8, Color::white)) &&
              (! square_attacked(Position::D8, Color::white)))
            move_list.moves.emplace_back(Position::E8, Position::C8, Piece::none, Piece::none, MoveFlag::castle);
        }
      }
    }

    // Sliders
    for (auto piece : sliders[static_cast<int>(side)]) {
      for (auto sq : bitboards[piece.value]) {
        Bitboard attacks;
        switch (piece.value) {
        case Piece::WR: case Piece::BR:
          attacks = get_rook_attacks(sq, bb_sides[static_cast<int>(Color::both)]);
          break;
        case Piece::WB: case Piece::BB:
          attacks = get_bishop_attacks(sq, bb_sides[static_cast<int>(Color::both)]);
          break;
        case Piece::WQ: case Piece::BQ:
          attacks = get_queen_attacks(sq, bb_sides[static_cast<int>(Color::both)]);
          break;
        }
        attacks &= ~ bb_sides[static_cast<int>(side)];
        for (auto t_sq : attacks) {
          auto t_piece = pieces[t_sq];
          move_list.moves.emplace_back(sq, t_sq, t_piece);
        }
      }
    }

    // Non-sliders
    for (auto piece : non_sliders[static_cast<int>(side)]) {
      for (auto sq : bitboards[piece.value]) {
        auto bb = [=]() {
          switch(piece.value) {
          case Piece::WN: case Piece::BN:
            return knight_moves[sq];
          case Piece::WK: case Piece::BK:
            return king_moves[sq];
          default:
            throw std::runtime_error("unreachable");
          }
        }();
        // Take moves bitboard and filter out side's pieces
        auto iterator = Bitboard(bb & ~ bb_sides[static_cast<int>(side)]);
        for (auto t_sq : iterator) {
          auto t_piece = pieces[t_sq];
          move_list.moves.emplace_back(sq, t_sq, t_piece);
        }
      }
    }

    return move_list;
  }

  std::vector<chess::Move> Board::generate_legal_moves() const {
    auto move_list = generate_all_moves();
    auto board_copy = *this;
    std::vector<chess::Move> moves;
    for (auto& mv : move_list.moves) {
      if (! board_copy.make_move(mv))
        continue;
      moves.push_back(mv);
      board_copy.undo_move();
    }
    return moves;
  }

};
