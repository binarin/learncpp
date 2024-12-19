#include "lib/lib.hpp"
#include <iostream>
#include <iterator>
#include <print>
#include <regex>
#include <string>

using num_t = int64_t;
using alternatives_t = std::vector<std::string>;

alternatives_t parse_alternatives(const std::string& str) {
  alternatives_t result{};
  std::regex words_re{"\\w+"};
  auto search_begin = std::sregex_iterator(str.begin(), str.end(), words_re);
  auto search_end = std::sregex_iterator();
  for (auto it = search_begin; it != search_end; ++it) {
    std::smatch match{*it};
    result.push_back(match.str());
  }
  return result;
}

num_t count_possibilities(const alternatives_t &alts, std::string_view s) {
  std::vector<num_t> multipliers(s.size() + 1);
  multipliers[0] = 1;

  for (int i = 0; i < s.size(); ++i) {
    if (!multipliers[i]) {
      continue;
    }
    std::string_view sub_view{s};
    sub_view.remove_prefix(i);

    for (auto alt: alts) {
      if (sub_view.starts_with(alt)) {
        multipliers[i + alt.size()] += multipliers[i];
      }
    }
  }

  return multipliers.back();
}

int main(int argc, char **argv) {
  std::vector<std::string> input = read_all_lines();
  assert(input.size() > 2);

  auto alternatives = parse_alternatives(input[0]);

  num_t total_alternatives{}, designs_possible{};


  for (auto it = input.begin() + 2; it != input.end(); it++) {
    std::string s{*it};
    num_t possibilities = count_possibilities(alternatives, s);
    total_alternatives += possibilities;
    if (possibilities > 0) {
      ++designs_possible;
    }
  }
  std::println("Possible designs {}", designs_possible);
  std::println("All options {}", total_alternatives);
}

int main_1(int argc, char **argv) {
  std::string available_str{cin_line()};
  cin_line();

  std::ostringstream towel_re_alternatives{};
  std::regex_replace(std::ostreambuf_iterator<char>(towel_re_alternatives), available_str.begin(), available_str.end(), std::regex{", "}, "|");
  std::string towel_re_part{"(" + towel_re_alternatives.str() + ")"};
  std::regex valid_design_re{"^" + towel_re_part + "+$"};

  std::string line;
  int ctr{0};
  while (std::getline(std::cin, line)) {
    if (std::regex_match(line.begin(), line.end(), valid_design_re)) {
      std::cout << line << "\n";
      ++ctr;
    }
  }
  std::println("Possible count {}", ctr);
  return 0;
}
