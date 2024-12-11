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

[[maybe_unused]] constexpr uint32_t DEBUG_SLOW_RENDER{1 << 0};
[[maybe_unused]] constexpr uint32_t DEBUG_PRINT_TRAILHEAD_SCORE{1 << 1};
constexpr uint32_t DEBUG = 0;

template <uint32_t flags, typename T> constexpr void debug_if(T fn) {
  if constexpr (DEBUG & flags) {
    fn();
  }
}

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


std::pair<int, int> trailhead_score(Coord2D root, const HeightMap &heights) {
  std::set<Coord2D> visited{};
  std::queue<std::pair<Coord2D, int>> queue;
  std::set<Coord2D> queued_coords{};
  std::set<Coord2D> climaxes{};
  int score = 0;

  auto render_path = [&]() {
    static auto [minCoord, maxCoord] = bounding_rect(heights);
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
    std::println("\nScore: {}, rating: {}", score, climaxes.size());
    std::cout << std::endl;
  };

  queue.emplace(root, 0);
  while (!queue.empty()) {
    debug_if<DEBUG_SLOW_RENDER>([&](){
      render_path();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    });

    auto [curCoord, expectedHeight] = queue.front();
    queue.pop();
    queued_coords.erase(curCoord);

    visited.insert(curCoord);

    if (heights.at(curCoord) == 9) {
      climaxes.insert(curCoord);
      ++score;
      continue;
    }

    auto maybeAdd = [&](Coord2D c) {
      if (!heights.contains(c))
        return;
      // if (visited.contains(c))
      //   return;
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

  debug_if<DEBUG_SLOW_RENDER>([&]() {
    render_path();
    std::println("Done!");
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  });

  return {climaxes.size(), score};
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

  debug_if<DEBUG_PRINT_TRAILHEAD_SCORE>([&]() {
    std::println("Num trailheads {}", roots.size());
  });

  int total_score{0}, total_rating{0};
  for (auto root : roots) {
    auto [score, rating] = trailhead_score(root, heights);
    debug_if<DEBUG_PRINT_TRAILHEAD_SCORE>([&]() {
      std::println("Trailhead ({}, {}) score is {}, rating is {}", root.x, root.y, score, rating);
    });
    total_score += score;
    total_rating += rating;
  }
  std::println("Score all trailheads {}", total_score);
  std::println("Rating all trailheads {}", total_rating);

  return 0;
}
