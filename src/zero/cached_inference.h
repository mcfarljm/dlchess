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
    float policy_softmax_temp_;

  public:

    CachedInferenceModel(std::shared_ptr<InferenceModel> model,
                         std::shared_ptr<Encoder> encoder,
                         int cache_size,
                         float policy_softmax_temp,
                         bool disable_underpromotion) :
    model_(std::move(model)), encoder_(encoder), cache_(cache_size),
    policy_softmax_temp_(policy_softmax_temp), disable_underpromotion_(disable_underpromotion) {}

    // Get current size of cache
    int cache_size() const {
      return cache_.set_.size();
    }

    // Get a neural network result, possibly using the cache.
    //
    // Returns true if there was a cache hit.  Value and prior data are updated as inplace arguments.

    // Todo: To return move_priors, may be better to use a pointer into the cache map.
    // After the data have been inserted, then use &(cache_.map_[hash]).  This avoids
    // having to copy the data.  Note that if multiple threads are ever used, will need
    // to review this for safety.
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

};


#endif // CACHED_INFERENCE_H
