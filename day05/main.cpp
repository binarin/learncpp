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
#include <set>
#include "lib/lib.hpp"

int main(int argc, char **argv) {
  std::string line{};
  std::map<int, std::set<int>> dependencies{};
  std::vector<std::vector<int>> updates{};

  bool prolog{true};
  while (std::getline(std::cin, line)) {
    if (line == "") {
      prolog = false;
      continue;
    }
    if (prolog) {
      std::string goes_before, goes_after;
      std::istringstream stream{line};
      std::getline(stream, goes_before, '|');
      std::getline(stream, goes_after, '|');
      dependencies[std::stoi(goes_after)].insert(std::stoi(goes_before));
    } else {
      std::vector<int> pages{};
      std::istringstream stream{line};
      std::string page;
      while (std::getline(stream, page, ',')) {
        pages.push_back(std::stoi(page));
      }
      updates.push_back(pages);
    }
  }
  std::cout << "Rules " << dependencies.size() << std::endl;

  std::cout << "Update " << updates.size() << std::endl;

  auto valid_update = [&](const std::vector<int> &pages) {
    std::set<int> seen_pages{};
    std::set<int> pages_set{pages.begin(), pages.end()};
    std::cout << "Checking pages: ";
    dump(pages);
    std::cout << std::endl;

    for (auto page : pages) {
      std::cout << "Page: " << page << "\nseen: ";
      dump(seen_pages);
      std::cout << "\nreqs:";

      std::set<int> relevant_deps{};
      dump(dependencies[page]);
            std::cout << "|||| ";
      std::ranges::set_intersection(dependencies[page], pages_set, std::inserter(relevant_deps, relevant_deps.begin()));

      dump(relevant_deps);
      std::cout << std::endl;

      if (!std::ranges::includes(seen_pages, relevant_deps)) {
        return false;
      }
      seen_pages.insert(page);
    }
    return true;
  };

  std::vector<std::vector<int>> valid_updates{}, invalid_updates{};

  for (auto u : updates) {
    if (valid_update(u)) {
      valid_updates.push_back(u);
    } else {
      invalid_updates.push_back(u);
    }
  }

  int result{0};
  for (auto u : valid_updates) {
    std::cout << "Adding " << u[u.size() / 2] << std::endl;
    result += u[u.size() / 2];
  }
  std::cout << "Valid count " << valid_updates.size() << std::endl;
  std::cout << "Valid sum " << result << std::endl;

  int result2{0};
  for (auto u : invalid_updates) {
    std::set<int> pages_set{u.begin(), u.end()};
    std::set<int> seen_pages;
    std::map<std::set<int>, int> queue;

    for (auto page : u) {
      std::set<int> relevant_deps{};
      std::ranges::set_intersection(
          dependencies[page], pages_set,
          std::inserter(relevant_deps, relevant_deps.begin()));
      queue[relevant_deps] = page;
    }

    std::vector<int> sorted_update{};
    while (true) {
      auto it = queue.find(seen_pages);
      if (it == queue.end()) {
        break;
      }
      sorted_update.push_back(it->second);
      seen_pages.insert(it->second);
      queue.erase(it);
    };

    std::cout << "Sorted ";
    dump(sorted_update);
    std::cout << std::endl;
    result2 += sorted_update[sorted_update.size() / 2];
  };
  std::cout << result2 << std::endl;
}
