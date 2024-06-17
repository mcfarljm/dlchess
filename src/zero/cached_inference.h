#ifndef CACHED_INFERENCE_H
#define CACHED_INFERENCE_H

#include <queue>
#include <unordered_set> // Todo: Temporary, for cache testing

#include "inference.h"
#include "../hashcat.h"


namespace zero {

  using namespace game_moves;

  struct fifo_map {
    int max_size_;
    std::queue<uint64_t> queue_;
    std::unordered_set<uint64_t> set_;

    fifo_map(int max_size) : max_size_(max_size) {}

    bool contains(uint64_t value) {
      return set_.contains(value);
    }

    void insert(uint64_t value) {
      while (set_.size() >= max_size_) {
        set_.erase(queue_.front());
        queue_.pop();
      }
      set_.insert(value);
      queue_.push(value);
    }

  };


  class CachedInferenceModel {

    std::shared_ptr<InferenceModel> model_;
    std::shared_ptr<Encoder> encoder_;

    fifo_map cache_;

    bool disable_underpromotion_;

  public:

    CachedInferenceModel(std::shared_ptr<InferenceModel> model,
                         std::shared_ptr<Encoder> encoder,
                         int cache_size,
                         bool disable_underpromotion) :
      model_(std::move(model)), encoder_(encoder), cache_(cache_size), disable_underpromotion_(disable_underpromotion) {}

    // Get current size of cache
    int cache_size() const {
      return cache_.set_.size();
    }

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

};


#endif // CACHED_INFERENCE_H
