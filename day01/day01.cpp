#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>
#include <ostream>
#include <vector>
#include <ranges>


int main(int argc, char **argv) {
  std::vector<int> l1 {};
  std::vector<int> l2 {};

  while (true) {
    int x;

    std::cin >> x;

    int y;
    std::cin >> y;

    if (std::cin.eof()) {
      break;
    }

    l1.push_back(x);
    l2.push_back(y);

    std::cout << "Line: " << x << " " << y << std::endl;
  }

  if (false) {
    std::sort(l1.begin(), l1.end());
    std::sort(l2.begin(), l2.end());

    auto diff = [](auto a, auto b) {
      return abs(a - b);
    };

    auto pairs = std::views::zip_transform(diff, l1, l2);
    auto result = std::reduce(pairs.begin(), pairs.end());
    std::cout << result << std::endl;
  }

  std::map<int, int> l2_counts {};
  for (auto i = l2.begin(); i != l2.end(); i++) {
    l2_counts[*i]++;
    std::cout << "l2 count " << *i << " " << l2_counts[*i] << std::endl;
  }

  int result2 { 0 };
  for (auto i = l1.begin(); i != l1.end(); i++) {
    std::cout << *i << " " << l2_counts[*i] << std::endl;
    result2 += *i * l2_counts[*i];
  }
  std::cout << result2 << std::endl;
}
