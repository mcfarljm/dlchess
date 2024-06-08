#ifndef TIME_MANAGEMENT_H
#define TIME_MANAGEMENT_H

#include <optional>

class TimeManager {
public:
  virtual float budget_ms(int time_left_ms, std::optional<int> inc_ms) = 0;
};

class AlphaZeroTimeManager : public TimeManager {
  float percentage_ = 5.0;
  float move_overhead_ms_ = 100.0;

public:
  AlphaZeroTimeManager() = default;
  AlphaZeroTimeManager(float percentage) : percentage_(percentage) {}
  float budget_ms(int time_left_ms, std::optional<int> inc_ms) {
    return (time_left_ms - move_overhead_ms_) * percentage_ * 0.01;
  }
};

#endif // TIME_MANAGEMENT_H
