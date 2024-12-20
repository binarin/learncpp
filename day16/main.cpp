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

using search_state_t = std::pair<Dir2D, Coord2D>;
using visited_t = std::map<search_state_t, int>;

std::map<search_state_t, int> search(const Grid& grid, Coord2D start, Dir2D start_orientation, Coord2D target) {
  PriorityQueue<search_state_t> queue{};
  visited_t visited{};

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

  return visited;
}

std::optional<int> best_score_at(const visited_t &visited, Coord2D target) {
  int best_cost{INT_MAX};
  for (auto d: Dir2D::all()) {
    search_state_t ss{d, target};
    if (!visited.contains(ss)) {
      return {};
    }
    int cost = visited.at(ss);
    if (cost < best_cost) {
      best_cost = cost;
    }
  }
  return best_cost;
}

int main(int argc, char **argv) {
  std::vector<std::string> input{read_all_lines()};
  auto [grid, start, target] = parse_input(input);
  auto visited = search(grid, start, Dir2D::Right, target);
  auto maybe_best_cost = best_score_at(visited, target);

  if (!maybe_best_cost) {
    throw std::runtime_error("Path not found");
  }
  int best_cost = maybe_best_cost.value();

  std::println("Best cost: {}", best_cost);
}
