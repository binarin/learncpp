#include "lib/lib.hpp"
#include "lib/coord.h"

#include <map>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>


using std::string;

class Cell {
  enum Variant {
    Wall = -1,
    Empty = -2,
  };

  int m_content;

  explicit Cell(int corruption_age) : m_content(corruption_age) {}

public:
  Cell(): m_content{Empty} {}
  static Cell wall() { return Cell{Wall}; }
  static Cell empty() { return Cell{Empty}; }
  static Cell falling_byte(int corruption_age) {
    return Cell{corruption_age};
  }

  bool is_passable() const { return m_content == Empty; }
  bool is_empty() const { return m_content == Empty; }
  bool is_wall() const { return m_content == Wall; }
  bool is_falling_byte() const { return m_content >= 0; }

  bool operator==(const Cell &other) { return m_content == other.m_content; }
};

template <class Cell, auto pad_constructor>
class PaddedGrid {
public:
  using value_t = Cell;

  PaddedGrid(PaddedGrid &&other)
  : m_top_left{other.m_top_left},
    m_bottom_right{other.m_bottom_right},
    m_map{std::move(other.m_map)}
  {}

  PaddedGrid(int width, int height)
      : m_top_left{0, 0}, m_bottom_right{width - 1, height - 1} {}

  Cell& operator[](int x, int y);
  Cell operator[] (int x, int y) const;

  bool is_within_explicit_bounds(Coord2D c) const;

  void insert(Coord2D c, Cell val);

  std::pair<Coord2D, Coord2D> bounds() const { return std::make_pair(m_top_left, m_bottom_right); }

  static std::pair<Coord2D, Coord2D> padded_bounds(const PaddedGrid& gr) {
    return std::make_pair(gr.m_top_left - Coord2D{1, 1}, gr.m_bottom_right + Coord2D{1, 1});
  }

private:
  static const Cell k_padding;


  Coord2D m_top_left{0, 0};
  Coord2D m_bottom_right{-1, -1};
  std::map<Coord2D, Cell> m_map{};
};

template <class Cell, auto pad_constructor>
const Cell PaddedGrid<Cell, pad_constructor>::k_padding{pad_constructor()};

template <class Cell, auto pad_constructor>
void PaddedGrid<Cell, pad_constructor>::insert(Coord2D c, Cell val) {
    m_map[c] = val;
    m_top_left.maybe_update_lower_boundary(c);
    m_bottom_right.maybe_update_upper_boundary(c);
}

template <class Cell, auto pad_constructor>
Cell& PaddedGrid<Cell, pad_constructor>::operator[](int x, int y) {
  Coord2D c{x, y};
  if (!is_within_explicit_bounds(c)) {
    throw std::runtime_error("Can't grow");
  }
  return m_map[c];
}

template <class Cell, auto pad_constructor>
Cell PaddedGrid<Cell, pad_constructor>::operator[](int x, int y) const {
  Coord2D c{x, y};
  if (m_map.contains(c)) {
    return m_map.at(c);
  }
  if (is_within_explicit_bounds(c)) {
    return Cell{};
  }
  return pad_constructor();
}

template <class Cell, auto pad_constructor>
bool PaddedGrid<Cell, pad_constructor>::is_within_explicit_bounds(Coord2D c) const {
  return c.x >= m_top_left.x && c.x <= m_bottom_right.x &&
         c.y >= m_top_left.y && c.y <= m_bottom_right.y;
}

using Grid = PaddedGrid<Cell, &Cell::wall>;

Grid parse_input(int width, int height, const std::string& str) {
  Grid result{width, height};
  std::regex re{"(\\d+),(\\d+)\n", std::regex::multiline};
  std::sregex_iterator search_begin{str.begin(), str.end(), re}, search_end{};

  int age{0};
  for (auto it = search_begin; it != search_end; ++it, ++age) {
    Cell content{Cell::falling_byte(age)};
    std::smatch match{*it};
    int x = std::stoi(match.str(1));
    int y = std::stoi(match.str(2));
    result.insert({x, y}, content);
  }

  return result;
}

template <class SomeGrid, typename BoundsFunctor, typename RenderFunctor>
std::string render_text_grid(SomeGrid &grid,
                             BoundsFunctor render_bounds,
                             RenderFunctor render_cell) {
  std::ostringstream s;
  auto [top_left, bottom_right] = render_bounds(grid);
  for (int y = top_left.y; y <= bottom_right.y; ++y) {
    for (int x = top_left.x; x <= bottom_right.x; ++x) {
      s << render_cell(x, y, grid[x, y]);
    }
    s << "\n";
  }
  return s.str();
}

class Visualization {
  std::string render_cell(int x, int y, Cell cell) {
    if (cell.is_falling_byte()) {
      return "#";
    }
    if (cell.is_wall()) {
      return "█";
    }
    return ".";
  }

public:
  auto render_cell_f() {
    return [&](int x, int y, Cell cell) {
      return render_cell(x, y, cell);
    };
  }
};


void render(const Grid& grid) {
  Visualization v;
  std::cout << render_text_grid(grid, &Grid::padded_bounds, v.render_cell_f());
}

namespace pathfind_1 {
  template <class Obs>
  concept HasObsStep = requires(Obs t) { t.step(int{}, Coord2D{}); };

  template <typename Obs> void obs_step(const Obs &obs, int step, Coord2D c) requires (!HasObsStep<Obs>) {};
  template <typename Obs> void obs_step(Obs &obs, int step, Coord2D c) requires HasObsStep<Obs> {
    obs.step(step, c);
  }

  class NullObs {};
  const NullObs null_observer{};

  template <class Obs> requires HasObsStep<Obs>
  class IsFullObserver;

  template <typename Obs = NullObs>
  void pathfind_1(Grid &grid, Obs &obs = const_cast<Obs&>(null_observer)) {
    // auto [start, target] = grid.bounds();
    obs_step(obs, 1, {2, 3});
  }
}

struct LogObs {
  void step(int step, Coord2D cur) {
    std::println("Starting step {} from {}", step, cur);
  }
};

template <> class pathfind_1::IsFullObserver<LogObs>;



int main(int argc, char **argv) {
  auto [width, height] = parse_n_numbers<int, 2>(cin_line());

  std::string input{read_whole_stdin()};
  auto grid = parse_input(width, height, input);
  // render(grid);
  pathfind_1::pathfind_1(grid);

  LogObs logger;
  pathfind_1::pathfind_1(grid, logger);
}
