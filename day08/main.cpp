#include "lib/coord.h"

#include "indicators/termcolor.hpp"

#include <algorithm>
#include <climits>
#include <iostream>
#include <map>
#include <print>
#include <set>
#include <string>
#include <vector>


int main(int argc, char **argv) {
  std::string line;

  std::map<char, std::vector<Coord2D>> antennas;
  std::map<Coord2D, char> antenna_at;

  Coord2D maxCoord{INT_MIN, INT_MIN};

  int y{0};
  while (std::getline(std::cin, line)) {
    int x = 0;
    std::cout << line << "\n";
    for (auto ch : line) {
      if (ch != '.') {
        antennas[ch].push_back({x, y});
        antenna_at[{x, y}] = ch;
      }
      ++x;
    }
    if (x - 1 > maxCoord.x) {
      maxCoord.x = x - 1;
    }
    ++y;
  }
  maxCoord.y = y - 1;

  std::cout << "\n";

  for (int y = 0; y <= maxCoord.y; y++) {
    for (int x = 0; x <= maxCoord.x; x++) {
      std::cout << (antenna_at.contains({x, y}) ? antenna_at[{x, y}] : '.' );
    }
    std::cout << "\n";
  }

  auto withinBound = [&](Coord2D c) {
    if (c.x > maxCoord.x || c.x < 0 || c.y > maxCoord.y || c.y < 0) {
      return false;
    }
    return true;
  };

  std::println("Max coord - {}, {}", maxCoord.x, maxCoord.y);
  std::println("Within bound test {}", true == withinBound({5, 5}));

  std::map<Coord2D, int> antinodes;

  // for (const auto &[freq, coords] : antennas) {
  //   std::println("Processing frequency {} - {} antennas", freq, coords.size());
  //   for (int i = 0; i < coords.size() - 1; i++) {
  //     for (int j = i + 1; j < coords.size(); j++) {
  //       int dx = coords[j].x - coords[i].x;
  //       int dy = coords[j].y - coords[i].y;

  //       Coord2D ic{coords[i].x - dx, coords[i].y - dy};
  //       Coord2D jc{coords[j].x + dx, coords[j].y + dy};

  //       if (withinBound(ic)) {
  //         antinodes[ic]++;
  //       }
  //       if (withinBound(jc)) {
  //         antinodes[jc]++;
  //       }
  //     }
  //   }
  // }

  for (const auto &[freq, coords] : antennas) {
    std::println("Processing frequency {} - {} antennas", freq, coords.size());
    for (int i = 0; i < coords.size() - 1; i++) {
      for (int j = i + 1; j < coords.size(); j++) {
        Coord2D j_coord = coords[j];
        Coord2D i_coord = coords[i];

        Coord2D delta = j_coord - i_coord;

        while (withinBound(i_coord)) {
          antinodes[i_coord] = 1;
          i_coord -= delta;
        }

        while (withinBound(j_coord)) {
          antinodes[j_coord] = 1;
          j_coord += delta;
        }
      }
    }
  }

  std::println("Antinodes count {}", antinodes.size());

  for (int y = 0; y <= maxCoord.y; y++) {
    for (int x = 0; x <= maxCoord.x; x++) {
      Coord2D cur{x, y};

      if (antenna_at.contains(cur)) {
        if (antinodes.contains({x, y})) {
          std::cout << termcolor::red << antenna_at[cur] << termcolor::reset;
        } else {
          std::cout << antenna_at[cur];
        }
      } else if (antinodes[{x, y}] > 0) {
        std::cout << '#';
      } else {
        std::cout << '.';
      }
    }
    std::cout << "\n";
  }
}
