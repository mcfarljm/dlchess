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
                     float& value);
  };

};


#endif // CACHED_INFERENCE_H
