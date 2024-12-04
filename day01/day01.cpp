#include <algorithm>
#include <iostream>
#include <ostream>
#include <vector>


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
}
