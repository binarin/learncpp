#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <ranges>

int main(int argc, char **argv) {
  std::istreambuf_iterator<char> it{std::cin}, end;
  std::string input{it, end};
  std::regex valid_mul{"mul\\(([0-9]{1,3}),([0-9]{1,3})\\)",
                       std::regex_constants::extended};

  auto input_begin{std::sregex_iterator(input.begin(), input.end(), valid_mul)};
  auto input_end = std::sregex_iterator{};
  // std::cout << "Found valid: " << std::distance(input_begin, input_end)
  //           << std::endl;
  int result{0};
  for (auto i = input_begin; i != input_end; i++) {
    std::smatch match = *i;
    result += std::stoi(match[1]) * std::stoi(match[2]);
  }
  std::cout << result << std::endl << "part2:\n";

  std::regex cond_mul_re{
      "(do)\\(\\)|(don't)\\(\\)|mul\\(([0-9]{1,3}),([0-9]{1,3})\\)",
      std::regex_constants::extended | std::regex_constants::multiline};
  bool mul_enabled = true;
  result = 0;
  for (auto i = std::sregex_iterator(input.begin(), input.end(), cond_mul_re);
       i != std::sregex_iterator{}; i++) {
    std::smatch match = *i;
    if (match.length(1) > 0) {
      mul_enabled = true;
    } else if (match.length(2) > 0) {
      mul_enabled = false;
    } else if (match.length(3) > 0 && mul_enabled) {
      result += std::stoi(match[3]) * std::stoi(match[4]);
    }
  }
  std::cout << result << std::endl;
}
