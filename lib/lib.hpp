#ifndef __LIB_HPP__
#define __LIB_HPP__

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <ranges>
#include <set>
#include <algorithm>
#include <iostream>
#include <iterator>
#include "indicators/block_progress_bar.hpp"
#include "indicators/cursor_control.hpp"

using namespace std::literals::string_literals;

template <class Container, typename T = typename Container::value_type> void dump(const Container &cont) {
  std::copy(cont.begin(), cont.end(), std::ostream_iterator<T>(std::cout, " "));
}

std::unique_ptr<indicators::BlockProgressBar, std::function<void(indicators::BlockProgressBar*)>> make_bar(const std::string &prefix, int max_progress);

inline void cls() {
  std::cout << "\033[H\033[2J";
}


#endif
