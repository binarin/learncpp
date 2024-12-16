#include "lib/lib.hpp"
#include <cctype>
#include <format>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <vector>
#include <print>

[[maybe_unused]] constexpr uint32_t DEBUG_SOLVE{1<<0};
[[maybe_unused]] constexpr uint32_t DEBUG_COMPARE_WITH_BRUTE{1 << 1};
constexpr uint32_t DEBUG = 0;

template <uint32_t flags, typename T> constexpr void debug_if(T fn) {
  if constexpr (DEBUG & flags) {
    fn();
  }
}

template<uint32_t flags, class... Args>
void println_if(std::format_string<Args...> fmt, Args&&... args) {
  if constexpr (DEBUG & flags) {
    std::cout << std::vformat(fmt.get(), std::make_format_args(args...)) << "\n";
  }
}

#define DDUMP(flags, expr) println_if<flags>("{}({}():{}) = {}", #expr, __FUNCTION__, __LINE__, (expr))

typedef int64_t Num;

Num gcd(Num a, Num b) {
  using std::min, std::max;

  while (a != b) {
    if ( a > b ) {
      a -= b;
    } else {
      b -= a;
    }
  }
  return a;
}

struct ClawMachine {
  Num a_dx, a_dy;
  Num b_dx, b_dy;
  Num target_x, target_y;

  std::optional<std::pair<Num, Num>> solve() {
    Num lcm_x = a_dx * b_dx / gcd(a_dx, b_dx);
    Num a_lcm_steps = lcm_x / a_dx;
    Num b_lcm_steps = lcm_x / b_dx;
    Num a_count{target_x / a_dx}, b_count{0};
    Num cur_x{a_count * a_dx}, cur_y{a_count * a_dy};

    Num remaining_steps_in_vicinity{a_lcm_steps};
    while (cur_x != target_x) {
      if (--remaining_steps_in_vicinity < 0) {
        return {};
      }
      while (cur_x > target_x) {
        cur_x -= a_dx;
        cur_y -= a_dy;
        --a_count;
      }
      while (cur_x < target_x) {
        cur_x += b_dx;
        cur_y += b_dy;
        ++b_count;
      }
    }

    debug_if<DEBUG_SOLVE>([&]() {
      std::println("Hit {} (Y={}/{}) at {}, {}", target_x, cur_y, target_y, a_count, b_count);
    });

    Num wanted_y_delta = target_y - cur_y;
    DDUMP(DEBUG_SOLVE, wanted_y_delta);
    if (wanted_y_delta == 0) {
      return std::make_pair(a_count, b_count);
    }

    // we can only decrease a_count in a_lcm_steps, while increasing b_count in b_lcm_steps
    // each such transformation changes Y with the following delta
    Num x_invariant_dy = b_lcm_steps * b_dy - a_lcm_steps * a_dy;
    DDUMP(DEBUG_SOLVE, x_invariant_dy);
    if (x_invariant_dy == 0) {
      return {};
    }

    // different signs
    if (x_invariant_dy * wanted_y_delta < 0) {
      return {};
    }

    if (wanted_y_delta % x_invariant_dy != 0) {
      println_if<DEBUG_SOLVE>("Target Y not reachable");
      return {};
    }

    Num invariant_applications = wanted_y_delta / x_invariant_dy;
    DDUMP(DEBUG_SOLVE, invariant_applications);

    a_count -= (invariant_applications * a_lcm_steps);
    b_count += (invariant_applications * b_lcm_steps);

    DDUMP(DEBUG_SOLVE, a_count);
    DDUMP(DEBUG_SOLVE, b_count);

    if (a_count < 0) {
      println_if<DEBUG_SOLVE>("Negative a_count {}", a_count);
      return {};
    }

    return std::make_pair(a_count, b_count);
  }

  std::optional<std::pair<Num, Num>> brute_force_solve() {
    std::optional<std::pair<Num, Num>> best_result{};
    for (int a_count = 0; a_count <= 200; ++a_count) {
      for (int b_count = 0; b_count <= 200; ++b_count) {
        if ((a_count * a_dx + b_count * b_dx == target_x ) && (a_count * a_dy + b_count * b_dy == target_y) ) {
          auto result = std::make_pair(a_count, b_count);
          if (!best_result) {
            best_result = result;
          } else {
            if (solution_cost(result) < solution_cost(best_result.value())) {
              best_result = result;
            }
          }
        }
      }
    }
    return best_result;
  }

  static Num solution_cost(std::pair<Num, Num> solution) {
    return solution.first * 3 + solution.second;
  }
};

template<>
struct std::formatter<ClawMachine, char> {
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && *it != '}') {
      throw std::format_error("ClawMachine format doesn't support args");
    }
    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(const ClawMachine& m, FmtContext& ctx) const {
    std::ostringstream out;
    out << "ClawMachine:\n";
    out << std::format("Button A: X+{}, Y+{}\n", m.a_dx, m.a_dy);
    out << std::format("Button B: X+{}, Y+{}\n", m.b_dx, m.b_dy);
    out << std::format("Prize: X={}, Y={}\n", m.target_x, m.target_y);
    return std::ranges::copy(std::move(out).str(), ctx.out()).out;
  }
};



struct MachineParser {
  struct Error : std::runtime_error {
    template<class... Args>
    Error(int line_no, int col_no, std::format_string<Args...> fmt, Args&&... args) :
      std::runtime_error(std::format("Error at line {}/col {} - ", line_no, col_no) + std::format(fmt, args...)) {}
  };

  std::string_view input, rest;

  int line_no{1}, col_no{0};

  MachineParser(std::string_view input) : input(input), rest(input) {}

  void update_counters(std::string_view s) {
    for (char c: s) {
      if ( c == '\n' ) {
        ++line_no;
        col_no = 0;
      }
    }
  }

  template<class... Args>
  [[noreturn]] void handle_error(std::format_string<Args...> fmt, Args&&... args) {
    throw Error(line_no, col_no, fmt, args...);
  }

  void p_string(std::string_view expected) {
    auto got = rest.substr(0, expected.size());
    if (got == expected) {
      update_counters(got);
      rest.remove_prefix(got.size());
    } else {
      handle_error("Expected '{}', got '{}'", expected, got);
    }
  }

  Num p_int() {
    int chars_used{};
    Num result{};
    while (isdigit(rest[chars_used])) {
      result = result * 10 + (rest[chars_used] - '0');
      ++chars_used;
    }
    if (!chars_used) {
      handle_error("No int found");
    }
    rest.remove_prefix(chars_used);
    col_no += chars_used;
    return result;
  }

  void p_whitespace() {
    int chars_used{};
    while (isspace(rest[chars_used])) {
      if (rest[chars_used] == '\n') {
        col_no = 0;
        ++line_no;
      }
      ++chars_used;
    }
    rest.remove_prefix(chars_used);
  }

  ClawMachine p_machine() {
    p_string("Button A: X+");
    Num a_dx = p_int();
    p_string(", Y+");
    Num a_dy = p_int();
    p_whitespace();
    p_string("Button B: X+");
    Num b_dx = p_int();
    p_string(", Y+");
    Num b_dy = p_int();
    p_whitespace();
    p_string("Prize: X=");
    Num target_x = p_int();
    p_string(", Y=");
    Num target_y = p_int();
    p_whitespace();
    return ClawMachine {
      .a_dx = a_dx,
      .a_dy = a_dy,
      .b_dx = b_dx,
      .b_dy = b_dy,
      .target_x = target_x,
      .target_y = target_y,
    };
  }

  std::vector<ClawMachine> p_machines() {
    std::vector<ClawMachine> result{};
    while (!rest.empty()) {
      result.push_back(p_machine());
    }
    return result;
  }
};

int main(int argc, char **argv) {
  std::string input(std::istreambuf_iterator<char>(std::cin), {});

  MachineParser parser{input};
  auto machines = parser.p_machines();
  std::println("Input size {}", machines.size());

  Num simple_cost{}, big_cost{};
  for (auto m : machines) {
    std::println("==============================\n{}", m);
    auto clever_solution = m.solve();

    debug_if<DEBUG_COMPARE_WITH_BRUTE>([&]() {
      auto brute_solution = m.brute_force_solve();
      if (clever_solution && brute_solution) {
        if (clever_solution != brute_solution) {
          auto br = brute_solution.value();
          auto cl = clever_solution.value();
          std::println("Solutions mismatch: brute ({},{}), clever: ({}, {})", br.first, br.second, cl.first, cl.second);
        }
      } else if (clever_solution) {
        auto cl = clever_solution.value();
        std::println("Only clever found ({}, {})", cl.first, cl.second);
      } else if (brute_solution) {
        auto br = brute_solution.value();
        std::println("Only brute found ({}, {})", br.first, br.second);
      }
    });

    clever_solution.and_then([&](auto pair) {
      Num cost{m.solution_cost(pair)};
      std::println("Solution: A={}, B={}\n  cost: {}", pair.first, pair.second, cost);
      simple_cost += cost;
      return std::optional<Num>{};
    });

    m.target_x += 10000000000000;
    m.target_y += 10000000000000;
    clever_solution = m.solve();
    clever_solution.and_then([&](auto pair) {
      Num cost{m.solution_cost(pair)};
      std::println("Solution: A={}, B={}\n  cost: {}", pair.first, pair.second, cost);
      big_cost += cost;
      return std::optional<Num>{};
    });
  }
  std::println("Simple cost: {}", simple_cost);
  std::println("Big cost: {}", big_cost);
}
