#include <array>

#include "movegen.h"
#include "board.h"

using game_moves::MoveFlag;

namespace movegen {
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

namespace board {
  movegen::MoveList Board::generate_all_moves() const {
    movegen::MoveList move_list;

    if (side == Color::white) {
      // Pawn non-captures:
      auto to_step1 = (bitboards[static_cast<int>(Piece::WP)] << 8) & (~ bb_sides[static_cast<int>(Color::both)]);
      auto to_step2 = (to_step1 << 8) & bitboard::BB_RANK_4 & (~ bb_sides[static_cast<int>(Color::both)]);

      for (auto to64 : Bitboard(to_step1))
        move_list.add_white_pawn_move(to64 - 8, to64, Piece::none);
      for (auto to64 : Bitboard(to_step2))
        move_list.moves.emplace_back(to64 - 2*8, to64, Piece::none, Piece::none, MoveFlag::pawnstart);

      // Pawn captures:
      auto to_cap_left = ((bitboards[static_cast<int>(Piece::WP)] & ~ bitboard::BB_FILE_A) << 7) & bb_sides[static_cast<int>(Color::black)];
      auto to_cap_right = ((bitboards[static_cast<int>(Piece::WP)] & ~ bitboard::BB_FILE_H) << 9) & bb_sides[static_cast<int>(Color::black)];
      for (auto to64 : Bitboard(to_cap_left))
        move_list.add_white_pawn_move(to64 - 7, to64, pieces[to64]);
      for (auto to64 : Bitboard(to_cap_right))
        move_list.add_white_pawn_move(to64 - 9, to64, pieces[to64]);

      // En passant captures:
      if (en_pas != Position::none) {
        auto ep_bb = Bitboard(1ULL << en_pas);
        auto ep_to_left = ((bitboards[static_cast<int>(Piece::WP)] & ~ bitboard::BB_FILE_A) << 7) & ep_bb;
        auto ep_to_right = ((bitboards[static_cast<int>(Piece::WP)] & ~ bitboard::BB_FILE_H) << 9) & ep_bb;

        for (auto to64 : Bitboard(ep_to_left))
          move_list.moves.emplace_back(to64 - 7, to64, Piece::none, Piece::none, MoveFlag::enpas);
        for (auto to64 : Bitboard(ep_to_right))
          move_list.moves.emplace_back(to64 - 9, to64, Piece::none, Piece::none, MoveFlag::enpas);
      }
    }
    else { // black

      // Pawn non-captures:
      auto to_step1 = (bitboards[static_cast<int>(Piece::BP)] >> 8) & (~ bb_sides[static_cast<int>(Color::both)]);
      auto to_step2 = (to_step1 >> 8) & bitboard::BB_RANK_5 & (~ bb_sides[static_cast<int>(Color::both)]);

      for (auto to64 : Bitboard(to_step1))
        move_list.add_white_pawn_move(to64 + 8, to64, Piece::none);
      for (auto to64 : Bitboard(to_step2))
        move_list.moves.emplace_back(to64 + 2*8, to64, Piece::none, Piece::none, MoveFlag::pawnstart);

      // Pawn captures:
      auto to_cap_left = ((bitboards[static_cast<int>(Piece::BP)] & ~ bitboard::BB_FILE_A) >> 9) & bb_sides[static_cast<int>(Color::white)];
      auto to_cap_right = ((bitboards[static_cast<int>(Piece::BP)] & ~ bitboard::BB_FILE_H) >> 7) & bb_sides[static_cast<int>(Color::white)];
      for (auto to64 : Bitboard(to_cap_left))
        move_list.add_white_pawn_move(to64 + 9, to64, pieces[to64]);
      for (auto to64 : Bitboard(to_cap_right))
        move_list.add_white_pawn_move(to64 + 7, to64, pieces[to64]);

      // En passant captures:
      if (en_pas != Position::none) {
        auto ep_bb = Bitboard(1ULL << en_pas);
        auto ep_to_left = ((bitboards[static_cast<int>(Piece::BP)] & ~ bitboard::BB_FILE_A) >> 9) & ep_bb;
        auto ep_to_right = ((bitboards[static_cast<int>(Piece::BP)] & ~ bitboard::BB_FILE_H) >> 7) & ep_bb;

        for (auto to64 : Bitboard(ep_to_left))
          move_list.moves.emplace_back(to64 + 9, to64, Piece::none, Piece::none, MoveFlag::enpas);
        for (auto to64 : Bitboard(ep_to_right))
          move_list.moves.emplace_back(to64 + 7, to64, Piece::none, Piece::none, MoveFlag::enpas);
      }
    }

    return move_list;
  }
};
