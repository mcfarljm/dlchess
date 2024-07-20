#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <chrono>

namespace utils {

  template <typename Out>
  void split_string(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
      *result++ = item;
    }
  }

  std::vector<std::string> split_string(const std::string &s, char delim);

  class Timer
  {
  public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
      return std::chrono::duration_cast<second_>
        (clock_::now() - beg_).count(); }

  private:
    using clock_ = std::chrono::steady_clock;
    using second_ = std::chrono::duration<double, std::ratio<1> >;
    std::chrono::time_point<clock_> beg_;
  };

  std::string format_seconds(double);

};

#endif // UTILS_H_
