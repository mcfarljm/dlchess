#ifndef TIME_MANAGEMENT_H
#define TIME_MANAGEMENT_H

#include <optional>

#include "chess/board.h"

class TimeManager {
public:
  virtual float budget_ms(int time_left_ms, std::optional<int> inc_ms, const
                          chess::Board& b) = 0;
};

class AlphaZeroTimeManager : public TimeManager {
  float percentage_ = 5.0;
  float move_overhead_ms_ = 250.0;

public:
  AlphaZeroTimeManager() = default;
  AlphaZeroTimeManager(float percentage) : percentage_(percentage) {}
  float budget_ms(int time_left_ms, std::optional<int> /* inc_ms */, const chess::Board& /* b */) override {
    return (static_cast<float>(time_left_ms) - move_overhead_ms_) * percentage_ * 0.01f;
  }
};


// Based on LC0 simple time manager.
class SimpleTimeManager : public TimeManager {
  float move_overhead_ms_ = 250.0;
  float base_pct_ = 1.4;
  float ply_pct_ = 0.049;
  float time_factor_ = 1.5;

public:
  SimpleTimeManager() = default;
  float budget_ms(int time_left_ms, std::optional<int> inc_ms, const chess::Board& b) override {
    const int increment = inc_ms ? std::max(0, *inc_ms) : 0;

    const float time_available = static_cast<float>(time_left_ms) - move_overhead_ms_;

    const float time_ratio = static_cast<float>(increment) / static_cast<float>(time_left_ms);

    // Increase percentage as ply count increases.
    float frac = (base_pct_ + static_cast<float>(b.total_moves) * ply_pct_) * 0.01f;

    // std::cout << "frac before: " << frac << "\n";
    // Increase fraction as ratio of increment to total time reaches equality
    frac += time_ratio * time_factor_;
    // std::cout << "frac after: " << frac << "\n";

    float time_budgeted = time_available * frac;

    time_budgeted = std::min(time_budgeted, time_available);

    return time_budgeted;
  }
};


#endif // TIME_MANAGEMENT_H
