#include <bitset>

#include "../chess/transform.h"
#include "encoder.h"

using chess::GRID_SIZE;

namespace {

  using chess::Transform;

  Transform choose_transform(const chess::Board& b) {
    Transform transform;
    if (b.side == chess::Color::black) {
      // Orient board towards side to move by performing vertical and horizontal flip.
      transform.set(static_cast<int>(chess::TransformType::flip_transform));
      transform.set(static_cast<int>(chess::TransformType::mirror_transform));
    }
    return transform;
  }

};


namespace zero {
  
  Tensor<float> SimpleEncoder::encode(const chess::Board& b) const {
    const int num_planes = en_passant_ ? 22 : 21;
    const std::vector<int64_t> board_tensor_shape = {1, num_planes, GRID_SIZE, GRID_SIZE};
    auto board_tensor = Tensor<float>(board_tensor_shape);

    Transform transform;
    if (orient_board_)
      transform = choose_transform(b);
    bool have_transform = transform.any();

    // First 12 planes encode piece occupation
    for (int plane=0; plane<chess::NUM_PIECE_TYPES_BOTH; ++plane) {
      auto piece_idx = [&](){
        // Note: this check may need to be updated if choose_transform is updated.
        if (have_transform) {
          // Arrange so that planes 0-5 have "our pieces" and 6-11 have "their pieces".
          if (plane < 6)
            return plane + 6;  // Swap white -> black pieces
          return plane - 6; // Swap black -> white pieces
        }
        return plane;
      }();

      auto bb = b.bitboards[static_cast<int>(piece_idx)];
      if (have_transform)
        bb = chess::transform_bitboard(bb, transform);
      for (auto sq : bb) {
        auto coords = chess::sq_to_rf(sq);
        board_tensor.at({0, plane, coords[0], coords[1]}) = 1.0;
      }
    }

    // Next two planes are flags for one and two repetitions.
    auto repetitions = b.repetition_count();
    if (repetitions >= 1)
      board_tensor.fill_channel(0, 12, 1.0);
    if (repetitions >= 2)
      board_tensor.fill_channel(0, 13, 1.0);

    // Color of side to move
    if (b.side == chess::Color::black)
      board_tensor.fill_channel(0, 14, 1.0);

    // Constant plane, to help with edge detection.  Was originally used for
    // total move count.
    board_tensor.fill_channel(0, 15, 1.0);

    // Castling
    if (b.side == chess::Color::white || ! orient_board_) {
      board_tensor.fill_channel(0, 16, b.castle_perm[castling::WK]);
      board_tensor.fill_channel(0, 17, b.castle_perm[castling::WQ]);
      board_tensor.fill_channel(0, 18, b.castle_perm[castling::BK]);
      board_tensor.fill_channel(0, 19, b.castle_perm[castling::BQ]);
    }
    else {
      // Orient permissions for black to move
      board_tensor.fill_channel(0, 16, b.castle_perm[castling::BK]);
      board_tensor.fill_channel(0, 17, b.castle_perm[castling::BQ]);
      board_tensor.fill_channel(0, 18, b.castle_perm[castling::WK]);
      board_tensor.fill_channel(0, 19, b.castle_perm[castling::WQ]);
    }

    // No progress count
    if (scale_move_count_)
      board_tensor.fill_channel(0, 20, static_cast<float>(b.fifty_move) / 100.0);
    else
      board_tensor.fill_channel(0, 20, static_cast<float>(b.fifty_move));

    // En passant
    if (en_passant_ && b.en_pas != chess::Position::none) {
      auto coords = chess::sq_to_rf(chess::transform_square(b.en_pas, transform));
      board_tensor.at({0, 21, coords[0], coords[1]}) = 1.0;
    }

    return board_tensor;
  }

  const std::array<int, 3> PRIOR_SHAPE = {73, 8, 8};

  // Given the board state, construct a map from legal moves to coordinates
  // associated with the tensor encoding.
  std::unordered_map<chess::Move, std::array<int,3>, chess::MoveHash>
  SimpleEncoder::decode_legal_moves(const chess::Board& b) const {
    constexpr int KNIGHT_BASE_PLANE = 56;
    constexpr int UNDERPROMOTION_BASE_PLANE = KNIGHT_BASE_PLANE + 8;

    Transform transform;
    if (orient_board_)
      transform = choose_transform(b);

    std::unordered_map<chess::Move, std::array<int,3>, chess::MoveHash> move_map;

    auto moves = b.generate_legal_moves();
    for (auto& mv : moves) {
      auto from_oriented = chess::transform_square(mv.from, transform);
      auto to_oriented = chess::transform_square(mv.to, transform);

      auto from_rank_file = chess::sq_to_rf(from_oriented);
      int plane;
      auto delta = to_oriented - from_oriented;
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
      move_map.emplace(mv, std::array<int,3>({plane, from_rank_file[0], from_rank_file[1]}));
    }
    return move_map;
  }

};
