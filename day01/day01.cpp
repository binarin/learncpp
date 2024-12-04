#include <algorithm>
#include <iostream>
#include <numeric>
#include <ostream>
#include <vector>
#include <ranges>


int main(int argc, char **argv) {
  std::vector<int> l1 {};
  std::vector<int> l2 {};

  while (!std::cin.eof()) {
    int x;

    std::cin >> x;
    l1.push_back(x);

    std::cin >> x;
    l2.push_back(x);
  }

  std::sort(l1.begin(), l1.end());
  std::sort(l2.begin(), l2.end());

  auto diff = [](auto a, auto b) {
    return abs(a - b);
  };

  auto pairs = std::views::zip_transform(diff, l1, l2);
  auto result = std::reduce(pairs.begin(), pairs.end());
  std::cout << result << std::endl;
}
