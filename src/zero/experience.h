#ifndef EXPERIENCE_H
#define EXPERIENCE_H

#include <vector>

#include <torch/torch.h>

namespace zero {

  class ExperienceCollector {
  public:
    std::vector<torch::Tensor> states;
    std::vector<torch::Tensor> visit_counts;
    std::vector<float> rewards;

  private:
    std::vector<torch::Tensor> current_episode_states;
    std::vector<torch::Tensor> current_episode_visit_counts;
  
  public:

    void begin_episode() {
      current_episode_states.clear();
      current_episode_visit_counts.clear();
    }

    void record_decision(torch::Tensor state, torch::Tensor visit_counts) {
      // Unsqueeze so that we get expected shape when concatenating
      current_episode_states.push_back(state.unsqueeze(0));
      current_episode_visit_counts.push_back(visit_counts.unsqueeze(0));
    }

    void complete_episode(float reward) {
      states.insert(states.end(), current_episode_states.begin(), current_episode_states.end());
      visit_counts.insert(visit_counts.end(), current_episode_visit_counts.begin(), current_episode_visit_counts.end());
      rewards.insert(rewards.end(), current_episode_states.size(), reward);

      // Clear current episode containers.
      current_episode_states.clear();
      current_episode_visit_counts.clear();
    }

    /// Append data from other.
    // Todo: could consider using a std::make_move_iterator and moving the data
    // from other.  Not sure how much difference it would make, as the Tensors
    // being moved seem to be smart pointers.
    void append(ExperienceCollector& other) {
      states.insert(states.end(), other.states.begin(), other.states.end());
      visit_counts.insert(visit_counts.end(), other.visit_counts.begin(), other.visit_counts.end());
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
    }


  };

};



#endif // EXPERIENCE_H
