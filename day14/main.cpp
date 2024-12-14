#include "lib/lib.hpp"
#include "lib/coord.h"
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <print>

typedef int Num;

std::vector<Num> extract_signed_numbers(const std::string &s) {
  std::regex re{"(-?\\d+)"};
  std::vector<Num> result{};
  auto numbers_begin = std::sregex_iterator(s.begin(), s.end(), re);
  auto numbers_end = std::sregex_iterator();
  for (auto it = numbers_begin; it != numbers_end; ++it) {
    result.push_back(std::stol(it->str()));
  }
  return result;
}

struct Robot {
  Coord2D c;
  Coord2D v;
  Coord2D bounds;

  Coord2D coord_at(int seconds) {
    Coord2D result;
    result.x = c.x + v.x * seconds;
    while (result.x < 0) {
      result.x += bounds.x;
    }
    result.y = c.y + v.y * seconds;
    while (result.y < 0) {
      result.y += bounds.y;
    }
    result.x %= bounds.x;
    result.y %= bounds.y;
    return result;
  }

  std::optional<int> quadrant_at(int seconds) {
    Coord2D cur = coord_at(seconds);
    if (cur.x == bounds.x / 2 || cur.y == bounds.x / 2) {
      return {};
    }
    bool x_quad = cur.x > (bounds.x / 2);
    bool y_quad = cur.y > (bounds.y / 2);
    return ((x_quad << 0) & (y_quad << 1));
  }
};

int main(int argc, char **argv) {
  std::string line;
  std::getline(std::cin, line);
  auto whl = extract_signed_numbers(line);
  Num width{whl[0]}, height{whl[1]};
  std::println("Field size {}x{}", width, height);

  std::vector<Robot> robots;
  while (std::getline(std::cin, line)) {
    auto nums = extract_signed_numbers(line);
    robots.emplace_back(Coord2D{nums[0], nums[1]}, Coord2D{nums[2], nums[3]}, Coord2D{width, height});
  }

  std::vector<int> quad_count(4, 0);
  std::map<Coord2D, int> occupied{};
  for (auto robot : robots) {
    Coord2D after_100 = robot.coord_at(100);
    occupied[after_100]++;
    // std::println("{}", robot.coord_at(100));
    // robot.quadrant_at(100).and_then([&](int quadrant) {
    //   quad_count[quadrant]++;
    //   return std::optional<int>{};
    // });
  }

  for (int y = 0; y < height; ++y) {
    if (y == height / 2) {
      std::cout << "\n";
      continue;
    }
    for (int x = 0; x < width; ++x) {
      if (x == width / 2) {
        std::cout << ' ';
      } else if (occupied[{x, y}]) {
        std::cout << occupied[{x, y}];
      } else {
        std::cout << '.';
      }
    }
    std::cout << "\n";
  }
  dump(quad_count);
}
