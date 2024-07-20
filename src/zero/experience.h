#ifndef EXPERIENCE_H
#define EXPERIENCE_H

#include <vector>
#include <iterator>

#include "tensor.h"
#include "../pieces.h"

namespace zero {

  /// Two possible implementations:
  /// 1. Use a tensor that grows in first dimension with an append function
  /// 2. Use a vector<tensor>, and preferably std::move tensors into the vector
  /// when appending.
  ///
  /// The second method avoids needing to reallocate each time to keep the
  /// memory contiguous.
  class ExperienceCollector {
  public:
    std::vector<Tensor<float>> states;
    std::vector<Tensor<float>> visit_counts;
    std::vector<float> rewards;

  private:
    std::vector<Tensor<float>> current_episode_states;
    std::vector<Tensor<float>> current_episode_visit_counts;
    std::vector<chess::Color> current_episode_sides;
  
  public:

    void begin_episode() {
      current_episode_states.clear();
      current_episode_visit_counts.clear();
      current_episode_sides.clear();
    }

    void record_decision(Tensor<float>&& state, Tensor<float>&& visit_counts, chess::Color side) {
      current_episode_states.push_back(std::move(state));
      current_episode_visit_counts.push_back(std::move(visit_counts));
      current_episode_sides.push_back(side);
    }

    void complete_episode(float white_reward) {
      states.insert(states.end(),
                    std::make_move_iterator(current_episode_states.begin()),
                    std::make_move_iterator(current_episode_states.end()));
      visit_counts.insert(visit_counts.end(),
                          std::make_move_iterator(current_episode_visit_counts.begin()),
                          std::make_move_iterator(current_episode_visit_counts.end()));
      assert(current_episode_states.size() == current_episode_sides.size());
      for (const auto& side : current_episode_sides)
        rewards.push_back(side == chess::Color::white ? white_reward : -white_reward);

      // Clear current episode containers.
      current_episode_states.clear();
      current_episode_visit_counts.clear();
      current_episode_sides.clear();
    }

    /// Append data from other.
    void append(ExperienceCollector&& other) {
      states.insert(states.end(),
                    std::make_move_iterator(other.states.begin()),
                    std::make_move_iterator(other.states.end()));
      visit_counts.insert(visit_counts.end(),
                          std::make_move_iterator(other.visit_counts.begin()),
                          std::make_move_iterator(other.visit_counts.end()));
      rewards.insert(rewards.end(), other.rewards.begin(), other.rewards.end());
    }

    void serialize_binary(const std::string directory, const std::string label);

    void reset() {
      states.clear();
      visit_counts.clear();
      rewards.clear();

      // Should be redundant if complete_episode has been called...
      current_episode_states.clear();
      current_episode_visit_counts.clear();
      current_episode_sides.clear();
    }


  };

};



#endif // EXPERIENCE_H
