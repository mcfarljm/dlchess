#include "cached_inference.h"

namespace zero {

  const NetworkOutput& CachedInferenceModel::operator() (const chess::Board& game_board,
                                         bool& cache_hit) {
    // Check cache:

    // Note that the Board hash does not include repetition or fifty move count, which
    // are part of the NN encoding.  So we concatenate them in, following LC0.

    auto hash = game_board.hash;
    hash = utils::HashCat(hash, game_board.repetition_count());
    hash = utils::HashCat(hash, game_board.fifty_move);

    if (cache_.contains(hash)) {
      cache_hit = true;
      return cache_.map_.at(hash);
    }


    // Prepare input and call neural net to get result:
    auto state_tensor = encoder_->encode(game_board);

    auto outputs = model_->operator()(state_tensor);

    auto priors = &outputs[0]; // Shape: (1, 73, 8, 8)
    auto values = &outputs[1]; // Shape: (1, 1)

    const float value = values->at({0, 0});

    priors_type move_priors;
    auto move_coord_map = decode_legal_moves(game_board);
    for (const auto &[mv, coords] : move_coord_map) {
      // std::cout << "move prior coords: " << mv << ": " << coords << std::endl;
      if (disable_underpromotion_ && mv.is_underpromotion())
        continue;
      move_priors.emplace(mv, priors->at({0, coords[0], coords[1], coords[2]}));
    }

    if (! move_priors.empty()) {
      // Apply softmax
      // Following LC0, subtract off the maximum.  This shouldn't change the result, but
      // maybe it helps conditioning.
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
    const float psum = std::accumulate(move_priors.begin(), move_priors.end(), 0.0,
                                       [](float value, const priors_type::value_type& p)
                                       {return value + p.second;}
                                       );
    for (auto &[mv, p] : move_priors)
      p /= psum;


    // Insert results into cache:
    cache_hit = false;
    cache_.insert(hash, std::move(NetworkOutput(std::move(move_priors), value)));
    return cache_.map_.at(hash);
  }

};
