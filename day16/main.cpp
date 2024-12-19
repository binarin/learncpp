#include "lib/grid.hpp"
#include "lib/lib.hpp"
#include <bits/ranges_algo.h>
#include <list>
#include <numeric>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
#include <ranges>

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

template <typename SearchState>
struct PriorityQueue {
  std::set<std::pair<int, SearchState>> queue{};
  std::map<SearchState, int> current_cost{};

  bool empty() {
    return queue.empty();
  }

  void enqueue(int cost, SearchState search_state) {
    if (current_cost.contains(search_state)) {
      int cur_cost{current_cost[search_state]};
      if (cur_cost <= cost) {
        return;
      }
      queue.erase({cur_cost, search_state});
    }
    queue.insert({cost, search_state});
    current_cost[search_state] = cost;
  }

  std::pair<int, SearchState> dequeue() {
    assert(!queue.empty());

    auto it = queue.begin();
    auto result = *it;
    auto [cost, search_state] = result;
    queue.erase(it);

    assert(current_cost.contains(search_state));
    assert(current_cost[search_state] == cost);
    current_cost.erase(search_state);

    return result;
  }
};


std::pair<int, int> search(const Grid& grid, Coord2D start, Dir2D start_orientation, Coord2D target) {
  using search_state_t = std::pair<Dir2D, Coord2D>;
  PriorityQueue<search_state_t> queue{};
  std::map<search_state_t, int> visited{};

  search_state_t initial_search_state{start_orientation, start};
  queue.enqueue(0, initial_search_state);

  while (!queue.empty()) {
    auto [cur_cost, cur_search_state] = queue.dequeue();
    auto [cur_dir, cur_coord] = cur_search_state;

    assert(!visited.contains(cur_search_state));

    visited[cur_search_state] = cur_cost;

    std::vector<std::tuple<int, Dir2D, Coord2D>> candidates{
      {cur_cost + 1000, cur_dir.left(), cur_coord},
      {cur_cost + 1000, cur_dir.right(), cur_coord},
      {cur_cost + 2000, cur_dir.left().left(), cur_coord},
      {cur_cost + 1, cur_dir, cur_coord.in_dir(cur_dir)},
    };

    if (cur_coord == target) {
      candidates.pop_back();
    }

    for (auto cand: candidates) {
      auto [new_cost, new_dir, new_coord] = cand;
      search_state_t new_search_state{new_dir, new_coord};

      if (grid[new_coord].is_wall()) {
        continue;
      }

      if (visited.contains(new_search_state)) {
        continue;
      }

      queue.enqueue(new_cost, new_search_state);
    }
  }

  auto best_search_states = [&](Coord2D coord) {
    std::vector<search_state_t> result{};
    int best_cost{INT_MAX};
    for (auto d: Dir2D::all()) {
      search_state_t ss{d, coord};
      int cost = visited[ss];
      if (cost < best_cost) {
        best_cost = cost;
        result = {ss};
      } else if (cost == best_cost) {
        result.push_back(ss);
      }
    }
    return std::make_pair(best_cost, result);
  };

  auto [best_cost, best_states] = best_search_states(target);
  std::set<Coord2D> best_tiles{};

  while (!best_states.empty()) {
    auto ss = best_states.back();
    best_states.pop_back();

    best_tiles.insert(ss.second);

    auto [best_cost, candidates] = best_search_states(ss.second);
    if (best_cost == 0) {
      continue;
    }

    for(search_state_t c: candidates) {
      auto [dir, coord] = c;
      best_states.push_back({dir, coord.in_dir(dir.reverse())});
    }
  }

  return {best_cost, best_tiles.size()};
}

int main(int argc, char **argv) {
  std::vector<std::string> input{read_all_lines()};
  auto [grid, start, target] = parse_input(input);
  auto [best_cost, num_tiles] = search(grid, start, Dir2D::Right, target);

  std::println("Reachable: {}", best_cost);
  std::println("Tiles: {}", num_tiles);
}
