#include <bitset>

#include "encoder.h"

using squares::GRID_SIZE;

namespace {
  enum {
    no_transform,
    // Horizontal mirror, reverse_bits_in_bytes
    flip_transform,
    // Vertical mirror, reverse_bytes_in_bytes
    mirror_transform,
    // Diagonal tranpose A1 to H8, tranpose_bits_in_bytes
    tranpose_transform,
  };

  // Flip bitboard horizontally.
  inline uint64_t reverse_bits_in_bytes(uint64_t v) {
    v = ((v >> 1) & 0x5555555555555555ull) | ((v & 0x5555555555555555ull) << 1);
    v = ((v >> 2) & 0x3333333333333333ull) | ((v & 0x3333333333333333ull) << 2);
    v = ((v >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((v & 0x0F0F0F0F0F0F0F0Full) << 4);
    return v;
  }

  // Flip bitboard vertically.
  inline uint64_t reverse_bytes_in_bytes(uint64_t v) {
    v = (v & 0x00000000FFFFFFFF) << 32 | (v & 0xFFFFFFFF00000000) >> 32;
    v = (v & 0x0000FFFF0000FFFF) << 16 | (v & 0xFFFF0000FFFF0000) >> 16;
    v = (v & 0x00FF00FF00FF00FF) << 8 | (v & 0xFF00FF00FF00FF00) >> 8;
    return v;
  }

  const bitboard::Bitboard lhs {0x0F0F0F0F0F0F0F0FULL};
  const bitboard::Bitboard top_half {0xFFFFFFFF00000000ULL};
  // Upper-right triangle within the lower-right quadrant
  const bitboard::Bitboard lower_right_upper_tri {0xE0C08000ULL};

  std::bitset<4> choose_transform(const board::Board& b) {
    if (b.castle_perm.any())
      return std::bitset<4>{};

    std::bitset<4> transform;
    auto king_piece = (b.side == Color::white ? Piece::WK : Piece::BK);
    auto king_bb = b.bitboards[static_cast<int>(king_piece)];
    if ((king_bb & lhs).any()) {
      transform.set(flip_transform);
      king_bb = Bitboard{reverse_bits_in_bytes(king_bb.to_ullong())};
    }
    // If there are any pawns, only horizontal flip is valid.
    if ((b.bitboards[static_cast<int>(Piece::WP)] | b.bitboards[static_cast<int>(Piece::BP)]).any())
      return transform;

    if ((king_bb & top_half).any()) {
      transform.set(mirror_transform);
      king_bb = Bitboard{reverse_bytes_in_bytes(king_bb.to_ullong())};
    }

    // Now king is in the bottom right quadrant.
    if ((king_bb & lower_right_upper_tri).any())
      transform.set(tranpose_transform);

    // LC0 checks if the king is on the diagonal within the bottom right quadrant, and
    // if it is it applies further checks to constrain the transform.
    return transform;
  }

};


namespace zero {
  
  // Todo: restore const
  Tensor<float> SimpleEncoder::encode(const board::Board& b) /*const*/ {
    // Check transform to canonical representation.
    ++_num_calls;
    if (_transform_position) {
      auto transform = choose_transform(b);
      if (transform.any())
        ++_transform_count;
    }


    std::vector<int64_t> board_tensor_shape = {1, 21, GRID_SIZE, GRID_SIZE};
    auto board_tensor = Tensor<float>(board_tensor_shape);

    // First 12 planes encode piece occupation
    for (int piece_idx=0; piece_idx<pieces::NUM_PIECE_TYPES_BOTH; ++piece_idx) {
      for (auto sq : b.bitboards[static_cast<int>(piece_idx)]) {
        auto coords = squares::sq_to_rf(sq);
        board_tensor.at({0, piece_idx, coords[0], coords[1]}) = 1.0;
      }
    }

    // Next two planes are flags for one and two repetitions (this is my
    // understanding of the AlphaZero paper, Table S1; instead of coding as a
    // numerical constant, they show two planes for this).
    auto repetitions = b.repetition_count();
    if (repetitions >= 1)
      board_tensor.fill_channel(0, 12, 1.0);
    if (repetitions >= 2)
      board_tensor.fill_channel(0, 13, 1.0);

    // Color of side to move
    if (b.side == Color::black)
      board_tensor.fill_channel(0, 14, 1.0);

    // Constant plane, to help with edge detection.  Was originally used for
    // total move count.
    board_tensor.fill_channel(0, 15, 1.0);

    // Castling
    board_tensor.fill_channel(0, 16, b.castle_perm[castling::WK]);
    board_tensor.fill_channel(0, 17, b.castle_perm[castling::WQ]);
    board_tensor.fill_channel(0, 18, b.castle_perm[castling::BK]);
    board_tensor.fill_channel(0, 19, b.castle_perm[castling::BQ]);

    // No progress count
    board_tensor.fill_channel(0, 20, b.fifty_move);

    return board_tensor;
  }

  const std::array<int, 3> PRIOR_SHAPE = {73, 8, 8};

  // Given the board state, construct a map from legal moves to coordinates
  // associated with the tensor encoding.
  std::unordered_map<game_moves::Move, std::array<int,3>, game_moves::MoveHash>
  decode_legal_moves(const board::Board& b) {
    constexpr int KNIGHT_BASE_PLANE = 56;
    constexpr int UNDERPROMOTION_BASE_PLANE = KNIGHT_BASE_PLANE + 8;

    std::unordered_map<game_moves::Move, std::array<int,3>, game_moves::MoveHash> move_map;

    auto moves = b.generate_legal_moves();
    for (auto& mv : moves) {
      auto from_rank_file = squares::sq_to_rf(mv.from);
      int plane;
      auto delta = mv.to - mv.from;
      if (b.pieces[mv.from].is_knight()) {
        switch (delta) {
        case 17:
          plane = KNIGHT_BASE_PLANE; break;
        case 10:
          plane = KNIGHT_BASE_PLANE + 1; break;
        case -6:
          plane = KNIGHT_BASE_PLANE + 2; break;
        case -15:
          plane = KNIGHT_BASE_PLANE + 3; break;
        case -17:
          plane = KNIGHT_BASE_PLANE + 4; break;
        case -10:
          plane = KNIGHT_BASE_PLANE + 5; break;
        case 6:
          plane = KNIGHT_BASE_PLANE + 6; break;
        case 15:
          plane = KNIGHT_BASE_PLANE + 7; break;
        default:
          throw std::runtime_error("invalid knight move");
        }
      }
      else {  // not a knight
        // Find direction and number of squares moved
        int direction;
        int amount;
        if (delta % 8 == 0) {
          if (delta > 0) {
            // N
            direction = 0;
            amount = delta / 8;
          } else {
            // S
            direction = 1;
            amount = delta / -8;
          }
        }
        else if (delta % 9 == 0) {
          if (delta > 0) {
            // NE
            direction = 2;
            amount = delta / 9;
          } else {
            // SW
            direction = 3;
            amount = delta / -9;
          }
        }
        else if (delta % 7 == 0) {
          if (delta > 0) {
            // NW
            direction = 4;
            amount = delta / 7;
          } else {
            // SE
            direction = 5;
            amount = delta / -7;
          }
        }
        else {
          assert(delta < 8 && delta > -8);
          if (delta > 0) {
            // E
            direction = 6;
            amount = delta;
          } else {
            // W
            direction = 7;
            amount = -delta;
          }
        }
        assert(amount > 0 && amount < 8);
        plane = direction * 7 + amount - 1;
        // std::cout << "dir, amt, plane: " << direction << " " << amount << " " << plane << std::endl;
        assert(plane >= 0 && plane < KNIGHT_BASE_PLANE);

        // Check for underpromotion
        if (mv.promote.exists() && !mv.promote.is_queen()) {
          // Pick one of 9 planes based on 3 underpromotions in 3 directions
          int base = UNDERPROMOTION_BASE_PLANE;
          if (direction == 0 || direction == 1)
            base += 0;
          else if (direction == 2 || direction == 3)
            base += 3;
          else {
            assert(direction == 4 || direction == 5);
            base += 6;
          }
          if (mv.promote.is_knight())
            plane = base;
          else if (mv.promote.is_bishop())
            plane = base + 1;
          else {
            assert(mv.promote.is_rook());
            plane = base + 2;
          }
          assert(plane >= UNDERPROMOTION_BASE_PLANE &&
                 plane < UNDERPROMOTION_BASE_PLANE + 9);
        }
      }
      assert(plane >= 0 && plane < 73);
      move_map.emplace(std::make_pair(mv, std::array<int,3>({plane, from_rank_file[0], from_rank_file[1]})));
    }
    return move_map;
  }

};
