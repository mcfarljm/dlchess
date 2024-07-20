#ifndef CACHED_INFERENCE_H
#define CACHED_INFERENCE_H

#include <queue>
#include <unordered_map>

#include "inference.h"
#include "../hashcat.h"


namespace zero {

  using namespace game_moves;

  using priors_type = std::unordered_map<Move, float, MoveHash>;

  struct NetworkOutput {
    priors_type move_priors;
    float value;

    NetworkOutput(priors_type p, float v) : move_priors(std::move(p)), value(v) {}
  };

  template <class T>
  struct fifo_map {
    int max_size_;
    std::queue<uint64_t> queue_;
    std::unordered_map<uint64_t, NetworkOutput> map_;

    fifo_map(int max_size) : max_size_(max_size) {
      map_.reserve(max_size);
    }

    bool contains(uint64_t key) {
      return map_.contains(key);
    }

    void insert(uint64_t key, T value) {
      while (map_.size() >= max_size_) {
        map_.erase(queue_.front());
        queue_.pop();
      }
      map_.emplace(std::make_pair(key, std::move(value)));
      queue_.push(key);
    }

  };


  class CachedInferenceModel {

    std::shared_ptr<InferenceModel> model_;
    std::shared_ptr<Encoder> encoder_;

    fifo_map<NetworkOutput> cache_;

    bool disable_underpromotion_;
    float policy_softmax_temp_;

  public:

    CachedInferenceModel(std::shared_ptr<InferenceModel> model,
                         std::shared_ptr<Encoder> encoder,
                         int cache_size,
                         float policy_softmax_temp,
                         bool disable_underpromotion) :
      model_(std::move(model)), encoder_(std::move(encoder)), cache_(cache_size),
      policy_softmax_temp_(policy_softmax_temp), disable_underpromotion_(disable_underpromotion) {}

    // Get current size of cache
    size_t cache_size() const {
      return cache_.map_.size();
    }

    // Get a neural network result, possibly using the cache.
    const NetworkOutput& operator() (const board::Board& game_board, bool& cache_hit);
  };

};


#endif // CACHED_INFERENCE_H
