#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

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

};

#endif // UTILS_H_
