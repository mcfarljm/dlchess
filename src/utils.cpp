#include "utils.h"


namespace utils {

  std::vector<std::string> split_string(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split_string(s, delim, std::back_inserter(elems));
    return elems;
  }

  std::string format_seconds(double seconds) {
    static constexpr int minute = 60;
    static constexpr int hour = minute * 60;
    static constexpr int day = hour * 24;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);
    if (seconds < minute)
      ss << seconds << " s";
    else if (seconds < hour)
      ss << seconds / minute << " m";
    else if (seconds < (hour * 99))
      ss << seconds / hour << " h";
    else
      ss << seconds / day << " d";
    return ss.str();
  }

};
