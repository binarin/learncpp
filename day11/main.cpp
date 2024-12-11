#include "lib/lib.hpp"
#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator>
#include <list>
#include <print>
#include <queue>
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

struct CachedSearch {
  int max_age;
  std::map<std::tuple<Num, int>, Num> cache;
  CachedSearch(int max_age = 75) : max_age(max_age) {};
  Num count_stones(Num val, int age) {
    if (cache.contains({val, age})) {
      return cache[{val, age}];
    }
    std::string val_str;
    Num ret;
    if (age == max_age) {
      ret = 1;
    } else if (val == 0) {
      ret = count_stones(1, age + 1);
    } else if ((val_str = std::to_string(val)).size() % 2 == 0) {
      auto left = std::stol(val_str.substr(0, val_str.length() / 2));
      auto right = std::stol(val_str.substr(val_str.length() / 2));
      ret = count_stones(left, age + 1) + count_stones(right, age + 1);
    } else {
      ret = count_stones(val * 2024, age + 1);
    }
    cache[{val, age}] = ret;
    return ret;
  }
};



int main(int argc, char **argv) {
  std::string line;
  std::getline(std::cin, line);
  std::list<Num> input{};

  auto line_stream = std::istringstream{line};
  std::copy(std::istream_iterator<Num>{line_stream}, std::istream_iterator<Num>(), std::back_inserter(input));

  dump(input); std::cout << std::endl;

  constexpr int simulation_steps = 75;
  CachedSearch search{simulation_steps};

  Num total{};
  for (auto root : input) {
    Num root_stones = search.count_stones(root, 0);
    std::println("Root {} expanded to {} stones", root, root_stones);
    total += root_stones;
  }
  std::println("Total stones {}", total);

  return 0;
}
