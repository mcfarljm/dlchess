#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <chrono>
#include <list>
#include <mutex>
#include <condition_variable>

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

  template <typename T>
  class SyncQueue {
  public:
    void put(const T& val) {
      std::lock_guard<std::mutex> lock(mtx_);
      values_.push_back(val);
      cv_.notify_one();
    }
    void get(T& val) {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait(lock, [this]{ return ! values_.empty(); });
      val = values_.front();
      values_.pop_front();
    }
  private:
    std::mutex mtx_;
    std::condition_variable cv_;
    std::list<T> values_;
  };

};

#endif // UTILS_H_
