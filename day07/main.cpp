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
#include "indicators/block_progress_bar.hpp"
#include "indicators/cursor_control.hpp"

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
    std::string total_expr{std::format("{}", total)};
    using res_pair = std::pair<int64_t, std::string*>;

    res_pair total_debug = std::make_pair(total, &total_expr);

    int remainder = comb;
    for (auto num : std::ranges::drop_view(eq.operands, 1)) {
      int bit = remainder % 2;
      auto op = bit == 1
                ? [](res_pair& res, int64_t num) { res.first += num; *res.second += std::format(" + {}", num); }
                : [](res_pair& res, int64_t num) { res.first *= num; *res.second += std::format(" * {}", num); };
      remainder = remainder / 2;
      op(total_debug, num);
    }
    if (total_debug.first == eq.target) {
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
  // std::println("Possible combs total {}", possible_combinations); //
  int last_comb{0};
  for (int comb = 0; comb < possible_combinations; comb++) {
    last_comb = comb;
    int64_t total = eq.operands.front();
    std::string total_expr{std::format("{}", total)};
    using res_pair = std::pair<int64_t, std::string*>;

    res_pair total_debug = std::make_pair(total, &total_expr);

    int remainder = comb;
    for (auto num : std::ranges::drop_view(eq.operands, 1)) {
      int bit = remainder % 3;
      auto op =
        (bit == 0) ? [](res_pair& res, int64_t num) { res.first += num; *res.second += std::format(" + {}", num); } :
        (bit == 1) ? [](res_pair& res, int64_t num) { res.first *= num; *res.second += std::format(" * {}", num); } :
        [](res_pair& res, int64_t num) { res.first = std::stoll(std::format("{}{}", res.first, num)); *res.second += std::format(" || {}", num); };
      remainder = remainder / 3;
      op(total_debug, num);
    }
    if (total_debug.first == eq.target) {
      // std::println("Found combination ({}): {} = {}", comb, eq.target, *total_debug.second);
      return true;
    }
  }
  // std::println("Failed after trying the final {} comb", last_comb);
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

  using namespace indicators;

  auto restore_cursor = [](void* ) { show_console_cursor(true); };
  std::unique_ptr<void, decltype(restore_cursor)> cursor_guard{NULL, restore_cursor};

  indicators::BlockProgressBar bar {
    option::PrefixText{"Detecting equations (base 2) 👀 "},
    option::BarWidth{60},
    option::ForegroundColor{Color::yellow},
    option::ShowElapsedTime{true},
    option::ShowRemainingTime{true},
    option::MaxProgress{eqns.size()},
  };

  int64_t sum{0};
  for (auto eqn : eqns) {
    bar.tick();
    // dump_equation(eqn);
    if (is_equation_resolvable_2(eqn)) {
      sum += eqn.target;
    }
  }
  bar.mark_as_completed();

  std::println("Result {}", sum);
}
