#include "lib/coord.h"
#include <climits>
#include <format>
#include <iostream>
#include <istream>
#include <map>
#include <stdexcept>
#include <utility>

struct Map {
  enum class Cell {
    Wall,
    Empty,
    Box,
  };

  std::map<Coord2D, Cell> map;
  Coord2D minCoord{INT_MAX, INT_MAX}, maxCoord{INT_MIN, INT_MIN};

  static std::pair<Map, Coord2D> parse_map(std::istream &in) {
    std::string line;
    Map result{};
    Coord2D robot_pos{};
    result.minCoord = {0, 0};
    int y{0};
    while (std::getline(in, line) && (line != "")) {
      int x{0};
      for(char c: line) {
        switch (c) {
        case '.':
          result.map[{x, y}] = Cell::Empty;
          break;
        case '#':
          result.map[{x, y}] = Cell::Wall;
          break;
        case 'O':
          result.map[{x, y}] = Cell::Box;
          break;
        case '@':
          robot_pos = {x, y};
          break;
        default:
          throw std::runtime_error(std::format("Unknown char in map {} at {}", c, Coord2D{x, y}));
        }
        result.maxCoord.x = std::max(result.maxCoord.x, x);
        ++x;
      }
      result.maxCoord.y = y;
      ++y;
    }
    return std::make_pair(result, robot_pos);
  }

  void render() const {
    for(int y = minCoord.y; y <= maxCoord.y; ++y) {
      for (int x = minCoord.x; y <= maxCoord.x; x++) {

      }
      std::cout << "\n";
    }
  }
};

int main(int argc, char **argv) {
}
