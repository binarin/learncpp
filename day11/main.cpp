#include "lib/lib.hpp"
#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator>
#include <list>
#include <print>
#include <sstream>
#include <string>
#include <vector>

[[maybe_unused]] constexpr uint32_t DEBUG_SLOW_RENDER{1 << 0};
[[maybe_unused]] constexpr uint32_t DEBUG_PRINT_TRAILHEAD_SCORE{1 << 1};
constexpr uint32_t DEBUG = 0;

typedef int64_t Num;

template <uint32_t flags, typename T> constexpr void debug_if(T fn) {
  if constexpr (DEBUG & flags) {
    fn();
  }
}
int main(int argc, char **argv) {
  std::string line;
  std::getline(std::cin, line);
  std::list<Num> input{};

  auto line_stream = std::istringstream{line};
  std::copy(std::istream_iterator<Num>{line_stream}, std::istream_iterator<Num>(), std::back_inserter(input));
  dump(input);

  auto stones{input};
  for (int step = 0; step < 75; step++) {
    std::string stone_str;
    for (auto it{stones.begin()}; it != stones.end(); ++it) {
      if (*it == 0) {
        *it = 1;
      } else if ((stone_str = std::to_string(*it)).size() % 2 == 0) {
        auto left = std::stol(stone_str.substr(0, stone_str.length() / 2));
        auto right = std::stol(stone_str.substr(stone_str.length() / 2));
        stones.insert(it, left);
        *it = right;
      } else {
        *it *= 2024;
      }
    }
    // std::println("\n After {} blinks", step + 1);
    // dump(stones);
  }
  std::println("Stones after 75 blinks - {}", stones.size());

  return 0;
}
