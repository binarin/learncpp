#include "lib/coord.h"
#include "indicators/termcolor.hpp"
#include <algorithm>
#include <chrono>
#include <climits>
#include <iostream>
#include <map>
#include <print>
#include <queue>
#include <set>
#include <string>
#include <thread>

bool DEBUG = false;

typedef std::map<Coord2D, int> HeightMap;

std::tuple<Coord2D, Coord2D> bounding_rect(const HeightMap &heights) {
  Coord2D min{INT_MAX, INT_MAX}, max{INT_MIN, INT_MIN};
  std::ranges::for_each(heights, [&](auto pair) {
    auto [x, y] = pair.first;
    if (x < min.x)
      min.x = x;
    if (x > max.x)
      max.x = x;
    if (y < min.y)
      min.y = y;
    if (y > max.y)
      max.y = y;
  });
  return {min, max};
}


int trailhead_score(Coord2D root, const HeightMap &heights) {
  std::set<Coord2D> visited{};
  std::queue<std::pair<Coord2D, int>> queue;
  std::set<Coord2D> queued_coords{};
  std::set<Coord2D> climaxes{};

  auto [minCoord, maxCoord] = bounding_rect(heights);
  auto render_path = [&]() {
    using namespace termcolor;
    std::cout << "\033[H\033[2J";
    for (int y = minCoord.y; y <= maxCoord.y; y++) {
      for (int x = minCoord.x; x <= maxCoord.x; x++) {
        Coord2D cur{x, y};
        auto front = queue.front().first;
        auto height = heights.at(cur);

        auto color = reset;
        if (root == cur) {
          color = magenta;
        } else if (front == cur) {
          color = red;
        } else if (climaxes.contains(cur)) {
          color = cyan;
        } else if (visited.contains(cur)) {
          color = yellow;
        } else if (queued_coords.contains(cur)) {
          color = green;
        }
        std::cout << color << static_cast<char>(height + '0') << reset;
      }
      std::cout << "\n";
    }
    std::cout << std::endl;
  };

  queue.emplace(root, 0);
  while (!queue.empty()) {
    if (DEBUG) {
      render_path();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    auto [curCoord, expectedHeight] = queue.front();
    queue.pop();
    queued_coords.erase(curCoord);

    visited.insert(curCoord);

    if (heights.at(curCoord) == 9) {
      climaxes.insert(curCoord);
      continue;
    }

    auto maybeAdd = [&](Coord2D c) {
      if (!heights.contains(c))
        return;
      if (visited.contains(c))
        return;
      if (heights.at(c) != expectedHeight + 1)
        return;
      queue.emplace(c, expectedHeight + 1);
      queued_coords.insert(c);
    };

    maybeAdd(curCoord.down());
    maybeAdd(curCoord.up());
    maybeAdd(curCoord.left());
    maybeAdd(curCoord.right());
  }

  return climaxes.size();
}


int main(int argc, char **argv) {
  std::string line;

  std::map<Coord2D, int> heights;
  std::set<Coord2D> roots;

  int y = 0;
  while (std::getline(std::cin, line)) {
    int x = 0;
    for (char height : line) {
      if (height == '.') {
        height = -1;
      } else {
        height -= '0';
      }
      heights[{x, y}] = height;
      if (height == 0) {
        roots.insert({x, y});
      }
      ++x;
    }
    ++y;
  }

  int total{0};
  if (DEBUG) std::println("Num trailheads {}", roots.size());
  for (auto root : roots) {
    int score = trailhead_score(root, heights);
    if (DEBUG) std::println("Trailhead ({}, {}) score is {}", root.x, root.y, score);
    total += score;
  }
  std::println("Sum all trailheads {}", total);

  return 0;
}
