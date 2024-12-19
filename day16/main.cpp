#include "lib/grid.hpp"
#include "lib/lib.hpp"
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

class Cell {
  enum Value {
    Empty, Wall
  } m_val;

  Cell(Value v): m_val(v) {}

public:
  static Cell out_of_bounds() {
    return wall();
  }

  Cell() : m_val{Empty} {}
  static const Cell empty() { return Cell(Empty); }
  static const Cell wall() { return Cell(Wall); }

  bool is_wall() const { return m_val == Wall; }
  bool is_empty() const { return m_val == Empty; }
};

using Grid = grid::Grid<Cell>;

std::tuple<Grid, Coord2D, Coord2D> parse_input(const std::vector<std::string> &input) {
  assert(input.size() > 3);
  assert(input[0].size() > 3);
  for (auto in: input) {
    assert(in.size() == input[0].size());
  }

  std::optional<Coord2D> start{}, target{};
  Grid grid(input[0].size(), input.size());

  for (int y = 0; y < input.size(); ++y) {
    for (int x = 0; x < input[y].size(); ++x) {
      Coord2D c{x, y};
      switch (input[y][x]) {
      case '#': grid.set(c, Cell::wall()); break;
      case '.': break;
      case 'S': start = c; break;
      case 'E': target = c; break;
      default:
        throw std::runtime_error(std::format("Unexpected char {}", static_cast<char>(input[y][x])));
      }
    }
  }
  if (!start) throw std::runtime_error("No start found");
  if (!target) throw std::runtime_error("No target found");
  return {grid,  start.value(), target.value()};
}

int search(const Grid& grid, Coord2D start, Dir2D start_orientation, Coord2D target) {

  std::set<std::tuple<int, Dir2D, Coord2D>> queue{};
  std::set<Coord2D> visited{};

  queue.insert({0, start_orientation, start});

  while (!queue.empty()) {
    auto [cost, dir, coord] = *(queue.begin());
    queue.erase(queue.begin());

    // std::println("Visiting {} {} {}", coord, dir.str(), cost);
    visited.insert(coord);

    if ( coord == target ) {
      return cost;
    }

    std::vector<std::pair<int, Dir2D>> candidates{
      {0, dir},
      {1000, dir.left()},
      {1000, dir.right()},
      {2000, dir.left().left()},
    };

    for (auto cand: candidates) {
      auto [delta_cost, dir] = cand;
      // std::println("Considering {}", dir.str());
      Coord2D c{coord.in_dir(dir)};
      if (visited.contains(c)) {
        continue;
      }
      if (grid[c].is_wall()) {
        continue;
      }
      queue.insert({cost + delta_cost + 1, dir, c});
    }
  }

  return 0;
}

int main(int argc, char **argv) {
  std::vector<std::string> input{read_all_lines()};
  auto [grid, start, target] = parse_input(input);
  std::println("Reachable: {}", search(grid, start, Dir2D::Right, target));
}
