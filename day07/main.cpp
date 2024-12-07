#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <ranges>
#include <set>
#include "lib/lib.hpp"
#include <print>

struct Equation {
  long long target = 0;
  std::vector<long long> operands{};
};

Equation parse_equation(const std::string &line) {
  static std::regex num_re{"\\d+"};

  Equation result{};

  auto nums = std::sregex_iterator(line.begin(), line.end(), num_re);
  result.target = std::stoll(nums->str());
  ++nums;
  std::vector<int64_t> operands{};
  std::transform(nums, std::sregex_iterator(),
                 std::back_inserter(result.operands),
                 [](const std::smatch &s) { return std::stoll(s.str()); });

  return result;
}


bool is_equation_resolvable(const Equation &eq) {
  int num_insertion_points = eq.operands.size() - 1;
  int possible_combinations = 1 << num_insertion_points;
  // std::println("Possible combs total {}", possible_combinations); //
  // int last_comb{0};
  for (int comb = 0; comb < possible_combinations; comb++) {
    // last_comb = comb;
    int64_t total = eq.operands.front();

    int remainder = comb;
    for (auto num : std::ranges::drop_view(eq.operands, 1)) {
      int bit = remainder % 2;
      auto op = bit == 1
                ? [](int64_t total, int64_t num) { return total + num; }
                : [](int64_t total, int64_t num) {  return total * num; };
      remainder = remainder / 2;
      total = op(total, num);
    }
    if (total == eq.target) {
      // std::println("Found combination ({}): {} = {}", comb, eq.target, *total_debug.second);
      return true;
    }
  }
  // std::println("Failed after trying the final {} comb", last_comb);
  return false;
}

uint64_t pow_i(uint64_t b, uint64_t e) {
  if (e == 1) {
    return b;
  }
  if (e % 2 == 0) {
    return pow_i(b * b, e / 2);
  } else {
    return b * pow_i( b, e - 1 );
  }
}

bool is_equation_resolvable_2(const Equation &eq) {
  int num_insertion_points = eq.operands.size() - 1;
  int possible_combinations = pow_i(3, num_insertion_points);
  for (int comb = 0; comb < possible_combinations; comb++) {
    int64_t total = eq.operands.front();

    int remainder = comb;
    for (auto num : std::ranges::drop_view(eq.operands, 1)) {
      int bit = remainder % 3;
      auto op =
        (bit == 0) ? [](uint64_t total, int64_t num) { return total + num; } :
        (bit == 1) ? [](uint64_t total, int64_t num) { return total * num; } :
                     [](uint64_t total, int64_t num) { return static_cast<uint64_t>(std::stoll(std::format("{}{}", total, num))); };
      remainder = remainder / 3;
      total = op(total, num);
    }
    if (total == eq.target) {
      return true;
    }
  }
  return false;
}


void dump_equation(const Equation &eq) {
  std::cout << "Equation: " << eq.target << " = ";
  std::copy(eq.operands.begin(), eq.operands.end(),
            std::ostream_iterator<int64_t>(std::cout, " "));
  std::cout << "\n";
}

int main(int argc, char **argv) {
  std::string line;
  std::vector<Equation> eqns;
  while (std::getline(std::cin, line)) {
    eqns.push_back(parse_equation(line));
  }
  std::println("Total number of equations - {}", eqns.size());

  auto bar = make_bar("Detecting equations (base 2) 👀 "s, eqns.size());
  int64_t sum_1{0};
  for (auto eqn : eqns) {
    bar->tick();
    if (is_equation_resolvable(eqn)) {
      sum_1 += eqn.target;
    }
  }
  bar.reset();
  std::println("Result (part 1) {}", sum_1);

  auto bar_2 = make_bar("Detecting equations (base 2) 👀 "s, eqns.size());
  int64_t sum_2{0};
  for (auto eqn : eqns) {
    bar_2->tick();
    if (is_equation_resolvable_2(eqn)) {
      sum_2 += eqn.target;
    }
  }
  bar_2.reset();
  std::println("Result (part 2) {}", sum_2);
}
