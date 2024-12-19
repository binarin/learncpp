#include "lib/lib.hpp"
#include <iostream>
#include <iterator>
#include <print>
#include <regex>
#include <string>

int main(int argc, char **argv) {
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
}
