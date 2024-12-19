#pragma once

#include "indicators/block_progress_bar.hpp"

#include <algorithm>
#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <iterator>
#include <ostream>
#include <print>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::literals::string_literals;

template <class Container, typename T = typename Container::value_type> void dump(const Container &cont) {
  std::copy(cont.begin(), cont.end(), std::ostream_iterator<T>(std::cout, " "));
}

std::unique_ptr<indicators::BlockProgressBar, std::function<void(indicators::BlockProgressBar*)>> make_bar(const std::string &prefix, int max_progress);

inline void cls() {
  std::cout << "\033[H\033[2J";
}

inline std::string read_whole_stdin() {
  return std::string{std::istreambuf_iterator<char>(std::cin), std::istreambuf_iterator<char>()};
}

inline std::vector<std::string> read_all_lines() {
  std::vector<std::string> result{};
  std::string line;
  while (std::getline(std::cin, line)) {
    result.push_back(line);
  }
  return result;
}

template <typename NumType>
struct NumParser {
  static NumType parse(const std::string&) {}
};

template <>
struct NumParser<int> {
  static int parse(const std::string& s) {
    return int{std::stoi(s)};
  }
};

template <>
struct NumParser<long> {
  static long parse(const std::string& s) {
    return long{std::stol(s)};
  }
};

template <class Value>
inline std::vector<Value> parse_all_numbers(const std::string& str) {
  std::vector<Value> result{};

  std::regex re{"-?\\d+"};
  std::sregex_iterator search_begin{str.begin(), str.end(), re}, search_end{};

  for (auto it = search_begin; it != search_end; ++it) {
    std::smatch match = *it;
    result.push_back(NumParser<Value>::parse(match.str(0)));
  }
  return result;
}

template <class Value, auto num>
inline std::array<Value, num> parse_n_numbers(const std::string& str, bool ignore_overflow = false) {
  std::array<Value, num> result{};

  std::regex re{"-?\\d+"};
  std::sregex_iterator search_begin{str.begin(), str.end(), re}, search_end{};

  int found{};
  for (auto it = search_begin; it != search_end; ++it, ++found) {
    if (found > num && !ignore_overflow) {
      throw std::runtime_error(std::format("Expected only {} elements in input '{}'", num, str));
    }
    std::smatch match = *it;
    result[found] = NumParser<Value>::parse(match.str(0));
  }
  if (found < num) {
    throw std::runtime_error(std::format("Expected {} elements, found {}: input='{}'", num, found, str));
  }
  return result;
}

inline std::string cin_line() {
  std::string line;
  if (!std::getline(std::cin, line)) {
    throw std::runtime_error("Can't get input line");
  }
  return line;
}
