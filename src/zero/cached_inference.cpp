#include "cached_inference.h"

namespace zero {

  bool CachedInferenceModel::operator() (const board::Board& game_board,
                                         std::unordered_map<Move, float, MoveHash>& move_priors,
                                         float& value) {

    // Prepare input and call neural net to get result:
    auto state_tensor = encoder_->encode(game_board);

    auto outputs = model_->operator()(state_tensor);

    auto priors = &outputs[0]; // Shape: (1, 73, 8, 8)
    auto values = &outputs[1]; // Shape: (1, 1)

    value = values->at({0, 0});

    auto move_coord_map = decode_legal_moves(game_board);
    for (const auto &[mv, coords] : move_coord_map) {
      // std::cout << "move prior coords: " << mv << ": " << coords << std::endl;
      if (disable_underpromotion_ && mv.is_underpromotion())
        continue;
      move_priors.emplace(mv, priors->at({0, coords[0], coords[1], coords[2]}));
    }

    if (! move_priors.empty()) {
      // Apply softmax
      // Following LC0, subtract off the maximum.  This shouldn't change the
      // result, but maybe it helps conditioning.
      auto pmax = std::max_element
        (
         std::begin(move_priors), std::end(move_priors),
         [] (const auto& pair1, const auto& pair2) {
           return pair1.second < pair2.second;
         }
         )->second;

      for (auto &[mv, p]: move_priors)
        p = exp((p - pmax) / policy_softmax_temp_);
    }

    // Renormalize prior based on legal moves:
    float psum = std::accumulate(move_priors.begin(), move_priors.end(), 0.0,
                                 [](float value, const std::unordered_map<Move, float, MoveHash>::value_type& p) {return value + p.second;}
                                 );
    for (auto &[mv, p] : move_priors)
      p /= psum;

    // Check cache:

    // Note that the Board hash does not include repetition or fifty move count, which
    // are part of the NN encoding.  So we concatenate them in, following LC0.

    auto hash = game_board.hash;
    hash = utils::HashCat(hash, game_board.repetition_count());
    hash = utils::HashCat(hash, game_board.fifty_move);

    if (cache_.contains(hash))
      return true;
    else {
      cache_.insert(hash);
      return false;
    }
  }

};
