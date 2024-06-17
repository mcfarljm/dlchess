#ifndef CACHED_INFERENCE_H
#define CACHED_INFERENCE_H

#include "inference.h"


namespace zero {

  using namespace game_moves;

  class CachedInferenceModel {

    std::shared_ptr<InferenceModel> model_;
    std::shared_ptr<Encoder> encoder_;

    bool disable_underpromotion_;

  public:

    CachedInferenceModel(std::shared_ptr<InferenceModel> model,
                         std::shared_ptr<Encoder> encoder,
                         bool disable_underpromotion) :
      model_(std::move(model)), encoder_(encoder), disable_underpromotion_(disable_underpromotion) {}

    // Get a neural network result, possibly using the cache.
    //
    // Returns true if there was a cache hit.  Value and prior data are updated as inplace arguments.
    bool operator() (const board::Board& game_board,
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

      return false;
    }

  };

};


#endif // CACHED_INFERENCE_H
