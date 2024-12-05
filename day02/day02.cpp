#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <ranges>

bool report_safe(const std::vector<int> &report) {
  auto safety_func = (report[0] > report[1])
    ? [](int cur, int next) { auto delta = cur - next; return delta >= 1 && delta <= 3; }
    : [](int cur, int next) { auto delta = next - cur; return delta >= 1 && delta <= 3; };

  auto safe = std::views::zip_transform(safety_func,
                                        report,
                                        std::ranges::drop_view{report, 1});

  return std::all_of(safe.begin(), safe.end(), [](auto x) { return x; });
}

bool report_safe_dampened(const std::vector<int> &report) {
  for (int skip = 0; skip < report.size(); skip++) {
    auto copy = report;
    copy.erase(copy.begin() + skip);
    if (report_safe(copy)) {
      return true;
    }
  }
  return false;
}

int main(int argc, char **argv) {
  std::vector<std::vector<int>> reports{};

  std::string line;
  while (std::getline(std::cin, line)) {
    std::istringstream line_stream {line};
    std::vector<int> report{};
    std::copy(std::istream_iterator<int>(line_stream), std::istream_iterator<int>(), std::back_insert_iterator(report));
    // std::cout << "Line: " << line << " " << report.size() << std::endl;
    reports.push_back(report);
  }

  std::vector<std::vector<int>> safe_reports;
  std::copy_if(reports.begin(), reports.end(), std::back_inserter(safe_reports),
               &report_safe_dampened);
  std::cout << safe_reports.size() << std::endl;
}
