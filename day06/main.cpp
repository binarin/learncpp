#include "lib/lib.hpp"
#include <climits>
#include <format>
#include <utility>

enum class MapBlock {
  Empty,
  Obstruction,
  OutOfBounds,
};
using enum MapBlock;

enum class Direction : int {
  Left,
  Up,
  Right,
  Down,
};

using enum Direction;

typedef std::pair<int, int> Coord;
typedef std::map<std::pair<int, int>, MapBlock> Map;

Direction rotateRight(Direction d) {
  return Direction{(std::to_underlying(d) + 1) % (std::to_underlying(Down) + 1)};
}

void dumpCoord(const Coord c, std::string prefix = "") {
  std::cout << std::format("{}({}, {})\n", prefix == "" ? "" : prefix + ": ", c.first, c.second);
}


Coord stepInDirection(Coord c, Direction d) {
  switch (d) {
  case Direction::Left:
    c.first -= 1;
    break;
  case Direction::Right:
    c.first += 1;
    break;
  case Direction::Up:
    c.second -= 1;
    break;
  case Direction::Down:
    c.second += 1;
    break;
  }
  return c;
}

MapBlock getBlock(const Map &map, Coord coord) {
  if (map.contains(coord)) {
    return map.at(coord);
  }
  return OutOfBounds;
}

typedef void (*TraverseCallback)(Coord guardCoord, Direction guardDirection);

enum class TraverseResult {
  OK, LoopDetected
};

TraverseResult traverseMap(const Map &map, Coord guardCoord, Direction guardDirection, TraverseCallback cb = NULL) {
  std::set<std::pair<Coord,Direction>> visited{};

  while (getBlock(map, guardCoord) != OutOfBounds) {
    auto visit = make_pair(guardCoord, guardDirection);
    if (visited.contains(visit)) {
      return TraverseResult::LoopDetected;
    }
    visited.insert(visit);
    if (cb) {
      cb(visit.first, visit.second);
    }

    Coord next = stepInDirection(guardCoord, guardDirection);

    while (Obstruction == getBlock(map, next)) {
      guardDirection = rotateRight(guardDirection);
      next = stepInDirection(guardCoord, guardDirection);
    }

    guardCoord = next;
  }

  return TraverseResult::OK;
}


int main(int argc, char **argv) {
  Map map{};

  Coord guardCoord, initialGuardCoord;
  Direction guardDirection, initialGuardDirection;
  int x{0}, y{0};
  char c;
  while (std::cin.get(c)) {
    if (c == '\n') {
      x = 0;
      ++y;
      continue;
    }
    if (c == '#') {
      map[std::make_pair(x, y)] = Obstruction;
    } else if (c == '^') {
      initialGuardCoord = guardCoord = std::make_pair(x, y);
      guardDirection = initialGuardDirection = Up;
      map[std::make_pair(x, y)] = Empty;
    } else if (c == '.') {
      map[std::make_pair(x, y)] = Empty;
    }
    ++x;
  }

  int minX{INT_MAX}, maxX{INT_MIN}, minY{INT_MAX}, maxY{INT_MIN};
  for (auto [x, y] : std::views::keys(map)) {
    if (x < minX) minX = x;
    if (x > maxX) maxX = x;
    if (y < minY) minY = y;
    if (y > maxX) maxY = y;
  }
  std::cout << std::format("Map rectangle ({}, {})-({}, {})", minX, minY, maxX, maxY) << std::endl;
  std::cout << std::format("Initial guard coord ({}, {})\n", guardCoord.first, guardCoord.second);

  std::set<Coord> visited{};

  while (getBlock(map, guardCoord) != OutOfBounds) {
    // dumpCoord(guardCoord, "Visiting");
    visited.insert(guardCoord);

    Coord next = stepInDirection(guardCoord, guardDirection);
    // dumpCoord(guardCoord, "Next candiate");

    while (Obstruction == getBlock(map, next)) {
      guardDirection = rotateRight(guardDirection);
      next = stepInDirection(guardCoord, guardDirection);
    }

    guardCoord = next;
  }

  std::cout << std::format("Visited blocks: {}\n", visited.size());

  int possibleObstructions{0};
  for (auto extraObstacleCoord : visited
         | std::views::filter([&](const Coord& c) {return c != initialGuardCoord; }) ) {
    auto tmpMap = map;
    tmpMap[extraObstacleCoord] = Obstruction;
    // dumpCoord(extraObstacleCoord, "Putting obstacle");
    switch (traverseMap(tmpMap, initialGuardCoord, initialGuardDirection)) {
    case TraverseResult::LoopDetected:
      std::cout << "Loop\n";
      possibleObstructions++;
      break;
    case TraverseResult::OK:
      break;
    }
  }
  std::cout << std::format("Possible obstructions: {}\n", possibleObstructions);

}
