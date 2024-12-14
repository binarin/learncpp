#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <vector>
#include <print>

typedef int64_t Num;

struct ClawMachine {
  Num a_dx, a_dy;
  Num b_dx, b_dy;
  Num target_x, target_y;

  std::optional<std::pair<int, int>> solve() const {
    Num steps{100*100*100*100};
    Num a_count{b_dx * target_x / (a_dx*b_dx)}, b_count{0};
    Num start{a_count * a_dx};
    Num cur_x{start}, cur_y{a_count * a_dy};

    while (!(cur_x == target_x && cur_y == target_y)) {
      std::println("{}, {} - {}, {} - {}, {}", a_count, b_count, cur_x, cur_y, a_count * a_dx + b_count * b_dx, a_count * a_dy + b_count * b_dy);

      if (cur_x == target_x) {
        cur_x -= a_dx;
        cur_y -= a_dy;
        --a_count;
      }
      if (cur_x > target_x) {
        Num delta = cur_x - target_x;
        Num repeat = delta / a_dx;
        if ( delta % a_dx != 0 ) {
          ++repeat;
        }
        cur_x -= repeat * a_dx;
        cur_y -= repeat * a_dy;
        a_count -= repeat;
      }

      while (cur_x < target_x) {
        Num delta = target_x - cur_x;
        Num repeat = delta / b_dx;
        if ( delta % b_dx != 0 ) {
          ++repeat;
        }
        cur_x += repeat * b_dx;
        cur_y += repeat * b_dy;
        b_count += repeat;
      }
      if (--steps < 0 || a_count < 0) {
        return {};
      }
    }
    return std::make_pair(a_count, b_count);
  }

  std::optional<std::pair<int, int>> solve2() {
    auto first = solve();
    auto second = ClawMachine{a_dy, a_dx, b_dy, b_dx, target_y, target_x}.solve(); // .transform([](auto pair) { return std::make_pair(pair.second, pair.first); });

    if (!first) {
      return second;
    }
    if (!second) {
      return first;
    }
    if (solution_cost(first.value()) <= solution_cost(second.value())) {
      return first;
    } else {
      std::println("Second solution");
      return second;
    }
  }

  std::optional<std::pair<int, int>> brute_force_solve() {
    std::optional<std::pair<int, int>> best_result{};
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

  static int solution_cost(std::pair<int, int> solution) {
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

  Num simple_cost{}, brute_cost{}, big_cost{};
  for (auto m : machines) {
    // m.target_x += m.a_dx * m.b_dx * m.a_dy * m.b_dy;
    // m.target_y += m.a_dx * m.b_dx * m.a_dy * m.b_dy;
    std::println("==============================\n{}", m);
    auto brute = m.brute_force_solve();
    auto simple = m.solve2();

    if (simple) {
      std::println("Found solution: {} x {} + {} x {} = {}", simple.value().first, m.a_dx, simple.value().second, m.b_dx, m.target_x);
      simple_cost += m.solution_cost(simple.value());
    }

    if (brute) {
      brute_cost += m.solution_cost(brute.value());
    }

    if (!simple && brute) {
      std::println("Only brute-force a solution: {} x {} + {} x {} = {}", brute.value().first, m.a_dx, brute.value().second, m.b_dx, m.target_x);
    }

    if (simple && brute && simple.value() != brute.value()) {
      std::println("Brute different solution: {} x {} + {} x {} = {}", brute.value().first, m.a_dx, brute.value().second, m.b_dx, m.target_x);
    }

    m.target_x += 10000000000000ll;
    m.target_y += 10000000000000ll;
    Num full_target_x{m.target_x};
    Num full_target_y{m.target_y};

    Num stride{m.a_dx    * m.b_dx * m.a_dy * m.b_dy};

    m.target_x %= stride;
    m.target_y %= stride;

    // m.target_y %= (m.a_dy * m.b_dy);
    // m.target_y += (m.a_dy * m.b_dy);

    std::println("!!!!!!!!!!!!!BIG!!!!!!!!!!!!!!!\n{}", m);
    auto big = m.solve();
    if (big) {
      std::println("Big solution: {} x {} + {} x {} = {}", big.value().first, m.a_dx, big.value().second, m.b_dx, m.target_x);
      big_cost += m.solution_cost(big.value());
    }
  }
  std::println("Simple cost: {}", simple_cost);
  std::println("Brute_cost: {}", brute_cost);
}
