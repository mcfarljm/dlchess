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

};
