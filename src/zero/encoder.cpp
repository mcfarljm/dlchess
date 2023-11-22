#include "encoder.h"

using squares::GRID_SIZE;
using namespace torch::indexing;


namespace zero {
  
  torch::Tensor SimpleEncoder::encode(const board::Board& b) const {
    auto board_tensor = torch::zeros({21, GRID_SIZE, GRID_SIZE});

    // First 12 planes encode piece occupation
    for (int piece_idx=0; piece_idx<pieces::NUM_PIECE_TYPES_BOTH; ++piece_idx) {
      for (auto sq : b.bitboards[static_cast<int>(piece_idx)]) {
        auto coords = squares::sq_to_rf(sq);
        board_tensor.index_put_({piece_idx, coords[0], coords[1]}, 1.0);
      }
    }

    // Next two planes are flags for one and two repetitions (this is my
    // understanding of the AlphaZero paper, Table S1; instead of coding as a
    // numerical constant, they show two planes for this).
    auto repetitions = b.repetition_count();
    if (repetitions >= 1)
      board_tensor.index_put_({12, Ellipsis}, 1.0);
    if (repetitions >= 2)
      board_tensor.index_put_({13, Ellipsis}, 1.0);

    // Color of side to move
    if (b.side == Color::black)
      board_tensor.index_put_({14, Ellipsis}, 1.0);

    // Total move count
    board_tensor.index_put_({15, Ellipsis}, b.total_moves);

    // Castling
    board_tensor.index_put_({16, Ellipsis}, b.castle_perm[castling::WK]);
    board_tensor.index_put_({17, Ellipsis}, b.castle_perm[castling::WQ]);
    board_tensor.index_put_({18, Ellipsis}, b.castle_perm[castling::BK]);
    board_tensor.index_put_({19, Ellipsis}, b.castle_perm[castling::BQ]);

    // No progress count
    board_tensor.index_put_({20, Ellipsis}, b.fifty_move);

    return board_tensor;
  }

  const std::array<int, 3> PRIOR_SHAPE = {8, 8, 73};

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
      move_map.emplace(std::make_pair(mv, std::array<int,3>({from_rank_file[0], from_rank_file[1], plane})));
    }
    return move_map;
  }

};
