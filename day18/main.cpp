#include "lib/lib.hpp"
#include "lib/coord.h"
#include "lib/color.h"

#include <chrono>
#include <map>
#include <ostream>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>


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

  bool is_passable(int age) const {
    if (is_empty() || (m_content >= 0 && age < m_content)) {
      return true;
    }
    return false;
  }
  bool is_empty() const { return m_content == Empty; }
  bool is_wall() const { return m_content == Wall; }
  bool is_falling_byte() const { return m_content >= 0; }
  int corruption_age() const { return m_content; }

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

  PaddedGrid() : m_top_left{0, 0}, m_bottom_right{-1, -1}, m_map{} {}

  void set(Coord2D c, Cell val);
  Cell operator[] (Coord2D c) const;

  bool is_within_explicit_bounds(Coord2D c) const;

  void insert(Coord2D c, Cell val);

  std::pair<Coord2D, Coord2D> bounds() const { return std::make_pair(m_top_left, m_bottom_right); }

  std::pair<Coord2D, Coord2D> padded_bounds() const {
    return std::make_pair(m_top_left - Coord2D{1, 1}, m_bottom_right + Coord2D{1, 1});
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
void PaddedGrid<Cell, pad_constructor>::set(Coord2D c, Cell val) {
  if (!is_within_explicit_bounds(c)) {
    throw std::runtime_error("Can't grow");
  }
  m_map[c] = val;
}

template <class Cell, auto pad_constructor>
Cell PaddedGrid<Cell, pad_constructor>::operator[](Coord2D c) const {
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

std::pair<Grid, std::vector<Coord2D>> parse_input(int width, int height, const std::string& str) {
  Grid grid{width, height};
  std::vector<Coord2D> bytes{};
  std::regex re{"(\\d+),(\\d+)\n", std::regex::multiline};
  std::sregex_iterator search_begin{str.begin(), str.end(), re}, search_end{};

  int age{1};
  for (auto it = search_begin; it != search_end; ++it, ++age) {
    Cell content{Cell::falling_byte(age)};
    std::smatch match{*it};
    int x = std::stoi(match.str(1));
    int y = std::stoi(match.str(2));
    grid.insert({x, y}, content);
    bytes.push_back({x, y});
  }
  return {std::move(grid), bytes};
}

template <class SomeGrid, typename RenderFunctor>
std::string render_text_grid(SomeGrid &grid,
                             std::pair<Coord2D, Coord2D> bounds,
                             RenderFunctor render_cell) {
  std::ostringstream s;
  auto [top_left, bottom_right] = bounds;
  for (int y = top_left.y; y <= bottom_right.y; ++y) {
    for (int x = top_left.x; x <= bottom_right.x; ++x) {
      s << render_cell(Coord2D{x, y}, grid[Coord2D{x, y}]);
    }
    s << "\n";
  }
  return s.str();
}

class Visualization {
protected:
  const Grid& m_grid;
  int m_age{};

  virtual std::string render_cell(Coord2D c, Cell cell) {
    if (cell.is_falling_byte()) {
      if (cell.is_passable(m_age)) {
        return "  ";
      } else {
        return "██";
      }
    }
    if (cell.is_wall()) {
      return "██";
    }
    return "  ";
  }

public:
  Visualization(const Grid& grid, int age = 0) : m_grid{grid}, m_age{age} {}
  void render() {
    std::cout << render_text_grid(m_grid, m_grid.padded_bounds(), [&](Coord2D c, Cell cell) {
      return render_cell(c, cell);
    });
  }
};

namespace pathfind_1 {
  template <typename Obs> struct Search;

  using search_queue_t = std::set<std::pair<int, Coord2D>>;

  template <class Obs> concept SearchObserverStart   = requires(Obs t) { t.start(int{}, Coord2D{}, Coord2D{}); };
  template <class Obs> concept SearchObserverVisit   = requires(Obs t) { t.visit(Coord2D{}, int{}); };
  template <class Obs> concept SearchObserverEnqueue = requires(Obs t) { t.enqueue(Coord2D{}, int{}); };
  template <class Obs> concept SearchObserverDelayed = requires(Obs t) { t.delayed(Coord2D{}, int{}, int{}); };
  template <class Obs> concept SearchObserverUnblock = requires(Obs t) { t.unblock(int{}, search_queue_t{}); };

  template <class Obs> concept AnyObserver  =
    SearchObserverStart<Obs>
    || SearchObserverVisit<Obs>
    || SearchObserverEnqueue<Obs>
    || SearchObserverDelayed<Obs>
    || SearchObserverUnblock<Obs>;

  template <class Obs> concept FullObserver =
    SearchObserverStart<Obs>
    && SearchObserverVisit<Obs>
    && SearchObserverEnqueue<Obs>
    && SearchObserverDelayed<Obs>
    && SearchObserverUnblock<Obs>;

  template <class Obs> requires FullObserver<Obs> class IsFullObserver;
  template <class Obs> requires SearchObserverStart<Obs> class IsStartObserver;
  template <class Obs> requires SearchObserverVisit<Obs> class IsVisitObserver;
  template <class Obs> requires SearchObserverEnqueue<Obs> class IsEnqueueObserver;
  template <class Obs> requires SearchObserverDelayed<Obs> class IsDelayedObserver;
  template <class Obs> requires SearchObserverUnblock<Obs> class IsUnblockObserver;

  template <class Obs> struct ObserverProxy {
    template <typename T> ObserverProxy(T&) {};
    template <typename ...Args> void start(Args ...args) { }
    template <typename ...Args> void visit(Args ...args) { }
    template <typename ...Args> void enqueue(Args ...args) { }
    template <typename ...Args> void delayed(Args ...args) { }
    template <typename ...Args> void unblock(Args ...args) { }
  };

  template <class Obs> requires (AnyObserver<Obs>) struct ObserverProxy<Obs> {
    Obs &obs;

    template <class TestObs = Obs, typename ...Args> requires std::same_as<TestObs, Obs> void start(Args ...args) {}
    template <class TestObs = Obs> requires (std::same_as<TestObs, Obs> && SearchObserverStart<TestObs>)
    void start(int fixed_age, Coord2D start, Coord2D target) {
      obs.start(fixed_age, start, target);
    }

    template <class TestObs = Obs, typename ...Args> requires std::same_as<TestObs, Obs> void visit(Args ...args) {}
    template <class TestObs = Obs> requires (std::same_as<TestObs, Obs> && SearchObserverVisit<TestObs>)
    void visit(Coord2D c, int step_no) {
      obs.visit(c, step_no);
    }

    template <class TestObs = Obs, typename ...Args> requires std::same_as<TestObs, Obs> void enqueue(Args ...args) {}
    template <class TestObs = Obs> requires (std::same_as<TestObs, Obs> && SearchObserverEnqueue<TestObs>)
    void enqueue(Coord2D c, int step_no) {
      obs.enqueue(c, step_no);
    }

    template <class TestObs = Obs, typename ...Args> requires std::same_as<TestObs, Obs> void delayed(Args ...args) {}
    template <class TestObs = Obs> requires (std::same_as<TestObs, Obs> && SearchObserverDelayed<TestObs>)
    void delayed(Coord2D c, int step_no, int expected_age) {
      obs.delayed(c, step_no, expected_age);
    }

    template <class TestObs = Obs, typename ...Args> requires std::same_as<TestObs, Obs> void unblock(Args ...args) {}
    template <class TestObs = Obs, typename Cont> requires (std::same_as<TestObs, Obs> && SearchObserverUnblock<TestObs>)
    void unblock(int age, Cont cont) {
      obs.unblock(age, cont);
    }
  };

  class NullObs {};
  [[maybe_unused]] const NullObs null_observer{};

  template <> class IsFullObserver<ObserverProxy<NullObs>>;

  template <typename Obs = NullObs>
  struct Search {
    Grid& m_grid;
    ObserverProxy<Obs> m_obs;
    Coord2D m_start, m_target;
    int m_age;
    int m_last_unblocked;
    using queue_t = std::set<std::pair<int, Coord2D>>;

    std::map<Coord2D,int> m_best_visit;
    std::set<std::pair<int, Coord2D>> m_queue;

    Search(Grid &grid, Obs &obs = const_cast<Obs&>(null_observer)) : m_grid{grid}, m_obs{obs} {}
    std::optional<int> operator()(int age, Coord2D start, Coord2D target) {
      m_start = start;
      m_target = target;
      m_last_unblocked = m_age = age;


      m_obs.start(age, start, target);

      m_best_visit.clear();
      m_queue.clear();
      m_queue.insert(std::make_pair(0, m_start));

      std::map<int, queue_t> delayed_queue{};

      while (!m_queue.empty()) {
        auto [steps, coord] = *m_queue.begin();
        if (coord == m_target) {
          return steps;
        }
        m_queue.erase(m_queue.begin());
        m_best_visit[coord] = steps;
        m_obs.visit(coord, steps);

        for(auto c: {coord.up(), coord.down(), coord.left(), coord.right()}) {
          if (m_best_visit.contains(c) && m_best_visit[c] <= steps + 1) {
            continue;
          }
          if (m_grid[c].is_passable(m_age)) {
            m_queue.emplace(steps + 1, c);
            m_obs.enqueue(c, steps + 1);
            continue;
          }
          if (m_grid[c].is_falling_byte() && m_grid[c].corruption_age() < m_age) {
            delayed_queue[m_grid[c].corruption_age()].emplace(steps + 1, c);
            m_obs.delayed(c, steps + 1, m_grid[c].corruption_age());
          }
        }

        if (m_queue.empty() && !delayed_queue.empty()) {
          auto [oldest_age, blocked_queue] = *(delayed_queue.rbegin());
          delayed_queue.erase(oldest_age);
          m_last_unblocked = oldest_age;
          m_age = oldest_age - 1;
          m_obs.unblock(m_age, blocked_queue);
          m_queue.merge(blocked_queue);
          --m_age;
        }
      }
      return {};
    }

  };
}

class VisualObserver : public Visualization {
  Coord2D m_current{}, m_target{};
  std::map<Coord2D, std::set<int>> m_queue{};
  std::set<Coord2D> m_visited{};
  std::set<Coord2D> m_delayed{};

  std::chrono::milliseconds m_visit_delay{100};
  std::chrono::milliseconds m_enqueue_delay{100};

public:
  using base = Visualization;
  VisualObserver(const Grid& grid, int age = 12) : base(grid, age) {}

  void slow_render(std::chrono::milliseconds delay) {
    cls();
    render();
    std::this_thread::sleep_for(delay);
  }

  void start(int fixed_age, Coord2D start, Coord2D target) {
    m_age = fixed_age;
    m_target = target;
    m_queue.clear();
    m_visited.clear();
  }

  void visit(Coord2D c, int steps) {
    m_visited.insert(c);
    m_queue[c].erase(steps);
    m_delayed.erase(c);
    m_current = c;
    slow_render(m_visit_delay);
  }

  void enqueue(Coord2D c, int steps) {
    m_queue[c].insert(steps);
  }

  void delayed(Coord2D c, int step, int expected_age) {
    m_delayed.insert(c);
  }

  void unblock(int new_max_age, const pathfind_1::search_queue_t& requeue) {
    m_age = new_max_age;
    for(auto req: requeue) {
      m_delayed.erase(req.second);
    }
  }

  std::string render_cell(Coord2D c, Cell val) {
    auto prev = base::render_cell(c, val);
    if (c == m_current) {
      return colored(termcolor::green, "🫅");
    } else if (m_visited.contains(c)) {
      return colored(termcolor::cyan, "＋");
    } else if (m_grid[c].is_falling_byte() && m_grid[c].is_passable(m_age)) {
      return colored(termcolor::green, "  ");
    } else if (m_delayed.contains(c)) {
      return colored(termcolor::yellow, "██");
    } else if (m_queue.contains(c) && !m_queue[c].empty()) {
      return colored(termcolor::cyan, "？");
    } else if (m_grid[c].is_wall()) {
      return colored(termcolor::bright_grey, "██");
    } else if (m_grid[c].is_falling_byte()) {
      return colored(termcolor::bright_grey, "██");
    } else {
      return prev;
    }
  }
};
template <> class pathfind_1::IsFullObserver<VisualObserver>;

struct LogObserver {
  void start(int age, Coord2D start, Coord2D target) {
    std::println("Starting search from {} to {} from an age {}", start, target, age);
  }
  void delayed(Coord2D c, int step, int expected_age) {
    std::println("Reached a falling-byte (blocks at {}) at coord {} (step {})", expected_age, c, step);
  }

  void unblock(int new_max_age, const pathfind_1::search_queue_t& requeue) {
    for(auto req: requeue) {
      std::println("Resuming search at {} (steps {})", req.second, req.first);
    }
  }

  void visit(Coord2D c, int steps) {
    std::println("Visiting {} (steps {})", c, steps);
  }

  void enqueue(Coord2D c, int steps) {
    std::println("Adding {} (steps {}) to the queue", c, steps);
  }
};
template <> class pathfind_1::IsFullObserver<LogObserver>;


int main(int argc, char **argv) {
  int p1_target_age = std::stoi(cin_line());
  auto [width, height] = parse_n_numbers<int, 2>(cin_line());

  std::string input{read_whole_stdin()};
  auto [grid, bytes] = parse_input(width, height, input);
  int p2_target_age = bytes.size();

  std::println("P1 t: {}, P2 t: {}", p1_target_age, p2_target_age);

  VisualObserver vis{grid, 12};
  vis.render();
  LogObserver logger{};

  auto search = pathfind_1::Search(grid, vis);
  auto [start, target] = grid.bounds();
  auto result = search(p2_target_age, start, target);
  if (result) {
    std::println("Reachable in {} (until {} falling byte - {})", result.value(), search.m_last_unblocked + 1, bytes[search.m_last_unblocked - 1]);
  } else {
    std::println("Not reachable");
  }
}
