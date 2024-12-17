#include "lib/coord.h"
#include "lib/debug.hpp"
#include "lib/lib.hpp"
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <istream>
#include <map>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>
#include <iterator>
#include <print>
#include <vector>

using namespace std::chrono_literals;

DEBUG_BIT(DEBUG_ITERATOR, 0)
DEBUG_BIT(DEBUG_SIMULATION, 1)
DEBUG_BIT(DEBUG_STENCIL, 2)
DEBUG_BIT(DEBUG_MERGE, 3)
DEBUG_BIT(DEBUG_SIMULATION_TOPLEVEL, 4)
DEBUG_BIT(DEBUG_SIMULATION_INVARIANTS, 5)
DEBUG_BIT(DEBUG_SIMULATION_PUSH_VERBOSE, 6)
SETUP_DEBUG(DEBUG_SIMULATION_INVARIANTS);
[[maybe_unused]] int simulation_delay = 100;

typedef int GPS;

static Dir2D parse_direction(char c) {
  switch (c) {
  case '^': return Dir2D::Up;
  case '>': return Dir2D::Right;
  case 'v': return Dir2D::Down;
  case '<': return Dir2D::Left;
  default: throw std::runtime_error(std::format("Unexpected robot direction '{}'", c));
  }
}

struct NarrowCell {
  enum Value : uint8_t {
    Wall, Empty, Box,
  } m_val;

  std::string str() const {
    switch (m_val) {
      case Value::Wall: return "#";
      case Value::Empty: return ".";
      case Value::Box: return "O";
    }
  }

  bool operator==(Value val) {
    return m_val == val;
  }

  using parse_result = std::vector<NarrowCell>;
  static std::optional<std::vector<NarrowCell>> parse_char(char c) {
    auto mk = [](Value val) {
      return std::vector<NarrowCell>{NarrowCell{val}};
    };
    switch (c) {
    case '#': return mk(Wall);
    case '.': return mk(Empty);
    case 'O': return mk(Box);
    default: return {};
    }
  }
};

struct WideCell {
  enum Value : int_fast8_t {
    Wall = 1, Empty, BoxLeft, BoxRight,
  };

  Value m_val;

  static constexpr WideCell wall();
  static constexpr WideCell empty();
  static constexpr WideCell box_left();
  static constexpr WideCell box_right();

  WideCell(): m_val(Wall) {};
  WideCell(Value val): m_val(val) {}

  std::string str() const {
    switch (m_val) {
    case Wall: return "█";
    case Empty: return ".";
    case BoxLeft:  return "[";
    case BoxRight: return "]";
    }
  }

  operator std::string() const { return str(); }

  operator int() const { return m_val; }

  bool operator==(int val) {
    return m_val == val;
  }

  using parse_result = std::vector<WideCell>;
  static std::optional<parse_result> parse_char(char c) {
    return NarrowCell::parse_char(c).transform([](NarrowCell::parse_result nar) {
      auto mk = [](Value val1, Value val2) {
        return std::vector<WideCell>{WideCell{val1}, WideCell{val2}};
      };
      switch (nar[0].m_val) {
      case NarrowCell::Wall: return mk(Wall, Wall);
      case NarrowCell::Empty: return mk(Empty, Empty);
      case NarrowCell::Box: return mk(BoxLeft, BoxRight);
      }
    });
  }
};

constexpr WideCell WideCell::wall() { return WideCell{WideCell::Value::Wall}; };
constexpr WideCell WideCell::empty() { return WideCell{WideCell::Value::Empty}; };
constexpr WideCell WideCell::box_left() { return WideCell{WideCell::Value::BoxLeft}; };
constexpr WideCell WideCell::box_right() { return WideCell{WideCell::Value::BoxRight}; };



template <typename Mod>
std::string colored(Mod mod, std::string_view s) {
  std::ostringstream os;
  os << termcolor::colorize << mod << s << termcolor::reset;
  return os.str();
}

template <class MapClass>
MapClass mk_map(typename MapClass::parse_input_t in) {
  MapClass result{};
  result.parse(in);
  return result;
}

template <class Cell> class Map2D {
protected:
  using cell_type_t = Cell;
  using cell_storage_t = std::map<Coord2D, Cell>;

  cell_storage_t m_map;
  Coord2D m_min_coord{INT_MAX, INT_MAX}, m_max_coord{INT_MIN, INT_MIN};

public:
  Cell operator[](Coord2D c) const {
    return m_map.at(c);
  }

  using stencil_t = std::set<Coord2D>;
  using slice_t = cell_storage_t;

  using parse_input_t = const std::vector<std::string>&;
  template<class MapClass> friend MapClass mk_map(typename MapClass::parse_input_t in);

  using annotation_t = std::function<std::string(const Cell&)>;
  using annotations_t = std::map<Coord2D, std::function<std::string(const Cell&)>>;

  Coord2D top_left_coord() const { return m_min_coord; }
  Coord2D bottom_right_coord() const { return m_max_coord; }

  void merge(slice_t &slice) {
    for(auto pair: slice) {
      m_map[pair.first] = pair.second;
    }
  }

  bool try_move_stencil(Coord2D source, Coord2D target, const stencil_t& stencil, Cell default_value, bool (*test)(Cell stencil_cell, Cell target_cell)) {
    std::vector<std::pair<Coord2D, Cell>> assignments{};
    std::set<Coord2D> clears{};

    DPRINT(DEBUG_STENCIL, "\nMoving stencil from {}", source);
    DDUMP(DEBUG_STENCIL, target);

    for (auto delta: stencil) {
      DPRINT(DEBUG_STENCIL, "\nTesting delta {}", delta);
      Coord2D cur_source = source + delta;
      Coord2D cur_target = target + delta;
      if (!m_map.contains(cur_source) || !m_map.contains(cur_target)) {
        throw std::runtime_error("Stencil out of bound");
      }
      Coord2D delta_in_target_frame = cur_target - source;
      bool target_covered_by_stencil = stencil.contains(delta_in_target_frame);
      Cell target_cell_value{target_covered_by_stencil ? default_value : m_map[cur_target]};

      DDUMP(DEBUG_STENCIL, cur_source);
      DDUMP(DEBUG_STENCIL, cur_target);
      DDUMP(DEBUG_STENCIL, delta_in_target_frame);
      DDUMP(DEBUG_STENCIL, target_covered_by_stencil);
      DDUMP(DEBUG_STENCIL, target_cell_value.str());

      if (!test(m_map[cur_source], target_cell_value)) {
        DPRINT(DEBUG_STENCIL, "Test failed for {}", cur_source);
        return false;
      }
      assignments.push_back({cur_target, m_map[cur_source]});
      clears.insert(cur_source);
    }

    for (auto upd: assignments) {
      auto [coord, value] = upd;
      clears.erase(coord);
      m_map[coord] = value;
    }
    for (auto clr: clears) {
      m_map[clr] = default_value;
    }
    return true;
  }

  virtual void render(const annotations_t &annotations) const {
    for (int y = m_min_coord.y; y <= m_max_coord.y; ++y) {
      for (int x = m_min_coord.x; x <= m_max_coord.x; ++x) {
        if (annotations.contains({x, y})) {
          std::cout << annotations.at({x, y})(m_map.at({x, y}));
        } else {
          std::cout << m_map.at({x, y}).str();
        }
      }
      std::cout << "\n";
    }
  }

protected:
  virtual char char_preprocessor(Coord2D coord, char c) { return c; }

  void parse(parse_input_t in) {
    m_min_coord = {0, 0};

    for (int y = 0; y < in.size(); y++) {
      int x{0};
      for (char c: in[y]) {
        auto parse_result = Cell::parse_char(char_preprocessor({x, y}, c));
        if (!parse_result) {
          throw std::runtime_error(std::format("Can't parse char '{}'", c));
        }
        for (Cell c: parse_result.value()) {
          m_map[{x, y}] = c;
          m_max_coord.maybe_update_upper_boundary({x, y});
          ++x;
        }
      }
    }
  }
};

template <class Cell>
struct Annotations {
  using ostream_mod = std::ostream& (&)(std::ostream&);
  using annotation_t = Map2D<Cell>::annotation_t;

  static annotation_t add_color(ostream_mod col) {
    return [&](const Cell& c) {
      return colored(col, c.str());
    };
  }

  static annotation_t colored_const(ostream_mod col, const std::string fixed_str) {
    return [=](const Cell&) {
      return colored(col, fixed_str);
    };
  }
#define ID(x) x
#define MK(color) \
  static annotation_t make_##color() { return add_color(termcolor::color); } \
  static annotation_t color##_const (const std::string fixed_str) { \
    return colored_const(termcolor::color, fixed_str); \
  }

  MK(red);
  MK(blue);
  MK(green);
  MK(cyan);
  MK(magenta);
  MK(bright_grey);
  MK(yellow);

#undef MK
};

class WideRobotMap : public Map2D<WideCell> {
public:
  void render(const annotations_t &annotations) const {
    annotations_t anns{annotations};
    if (!anns.contains(m_robot_coord)) {
      anns[m_robot_coord] = Annotations<WideCell>::cyan_const("@");
    }
    Map2D::render(anns);
  }

  Coord2D robot_coord() const {
    return m_robot_coord;
  }

  void move_robot(Coord2D coord) {
    if (!coord.within_bounds(m_min_coord, m_max_coord)) {
      throw std::runtime_error(std::format("Can't move robot to a position {} outside of bounds {}-{}", coord, m_min_coord, m_max_coord));
    }
    if (m_map[coord] != WideCell::Empty) {
      throw std::runtime_error(std::format("Can't move robot to a non-empty position {} containing '{}'", coord, m_map[coord].str()));
    }
    m_robot_coord = coord;
  }

  static GPS gps(Coord2D c) {
    return c.y * 100 + c.x;
  }

  GPS gps_all_boxes() const {
    GPS result{};
    for (auto pair: m_map) {
      if (pair.second == WideCell::BoxLeft) {
        result += gps(pair.first);
      }
    }
    return result;
  }


protected:
  Coord2D m_robot_coord;
  char char_preprocessor(Coord2D coord, char c) {
    if ( c == '@') {
      m_robot_coord = coord;
      return '.';
    }
    return c;
  }
};

struct WideSimulator {
  WideRobotMap map;
  std::vector<Dir2D> instructions;

  bool visualize{true}, collect_annotations{true};
  std::chrono::milliseconds visualisation_delay{150ms};

  WideRobotMap::annotations_t anns{}, static_annotations{};

  using Ann = Annotations<WideCell>;

  using stencil_t = WideRobotMap::stencil_t;

  bool simulation_step(Dir2D instr) {
    std::optional<stencil_t> pushable_stencil{};

    const Coord2D start{map.robot_coord()};
    const Coord2D target{start.in_dir(instr)};
    DPRINT(DEBUG_SIMULATION, "Trying to move from {} to {}", start, target);

    if (map[target] == WideCell::Empty) {
      map.move_robot(target);
      anns[start] = Ann::bright_grey_const(instr.str());
      anns[target] = Ann::green_const("@");
      return true;
    }

    if (map[target] == WideCell::Wall) {
      anns[start] = Ann::red_const(instr.str());
      anns[target] = Ann::make_red();
      return false;
    }

    bool moved = try_push_box(target, instr);
    DDUMP(DEBUG_SIMULATION, moved);

    if (!moved) {
      anns[start] = Ann::red_const(instr.str());
      return false;
    }
    map.move_robot(target);
    anns[start] = Ann::bright_grey_const(instr.str());
    anns[target] = Ann::green_const("@");
    return true;
  }

  void render_slice(WideRobotMap::slice_t &updates) {
    Coord2D tl{map.top_left_coord()}, br{map.bottom_right_coord()};
    for (int y = tl.y; y <= br.y; ++y) {
      for (int x = tl.x; x <= br.x; ++x) {
        std::print("{}", updates.contains({x, y}) ? updates[{x, y}].str() : " ");
      }
      std::println();
    }
  }

  bool try_push_box(Coord2D coord, Dir2D instr) {
    WideRobotMap::slice_t updates{};
    bool can_push{false};
    switch (instr.m_val) {
    case Dir2D::Up:
    case Dir2D::Down:
      can_push = try_push_box_v(coord, instr, updates);
      break;
    case Dir2D::Right:
    case Dir2D::Left:
      can_push = try_push_box_h(coord, instr, updates);
      break;
    }
    if (!can_push) {
      return false;
    }

    if constexpr ( DEBUG & DEBUG_SIMULATION ) {
      render_slice(updates);
    };

    if (collect_annotations) {
      for (auto pair : updates) {
        if (pair.second == WideCell::Wall || pair.second == WideCell::Empty) {
          anns[pair.first] = static_annotations[pair.first] = Ann::make_bright_grey();
        } else {
          static_annotations.erase(pair.first);
          anns[pair.first] = Ann::make_yellow();
        }
      }
    }

    ValidationResult before_merge, after_merge;
    if constexpr (DEBUG & DEBUG_SIMULATION_INVARIANTS) {
      before_merge = validate_map();
    }

    map.merge(updates);

    if constexpr (DEBUG & DEBUG_SIMULATION_INVARIANTS) {
      after_merge = validate_map();

      if (after_merge.box_count != before_merge.box_count
          || after_merge.wall_count != before_merge.wall_count
          || after_merge.empty_count != before_merge.empty_count
          || before_merge.broken_box_count > 0
          || after_merge.broken_box_count > 0) {
        std::println("Before merge: boxes {}, walls {}, empties {}, broken boxes {}", before_merge.box_count, before_merge.wall_count, before_merge.empty_count, before_merge.broken_box_count);
        std::println("After merge: boxes {}, walls {}, empties {}, broken boxes {}", after_merge.box_count, after_merge.wall_count, after_merge.empty_count, after_merge.broken_box_count);
        map.render(anns);
        throw std::runtime_error("INVARIANT");
      }

    }

    return true;
  }

  struct ValidationResult {
    int box_count{}, broken_box_count{}, wall_count{}, empty_count{};
  };
  ValidationResult validate_map() const {
    ValidationResult result{};
    Coord2D tl{map.top_left_coord()}, br{map.bottom_right_coord()};
    for (int y = tl.y; y <= br.y; ++y) {
      for (int x = tl.x; x <= br.x; ++x) {
        switch (map[{x, y}].m_val) {
        case WideCell::Wall: ++result.wall_count; break;
        case WideCell::Empty: ++result.empty_count; break;
        case WideCell::BoxLeft:
          if (map[Coord2D{x, y}.in_dir(Dir2D::Right)] == WideCell::BoxRight) {
            ++result.box_count;
          } else {
            ++result.broken_box_count;
          }
          break;
        case WideCell::BoxRight:
          if (map[Coord2D{x, y}.in_dir(Dir2D::Left)] != WideCell::BoxLeft) {
            ++result.broken_box_count;
          }
          break;
        }
      }
    }
    return result;
  }

  bool try_push_box_h(Coord2D coord, Dir2D instr, WideRobotMap::slice_t &update) {
    // XXX it really pushes half-boxes around
    DDUMP(DEBUG_SIMULATION, coord);

    const Coord2D target{coord.in_dir(instr)};
    DDUMP(DEBUG_SIMULATION, target);

    if (map[target] == WideCell::Wall) {
      DPRINT(DEBUG_SIMULATION, "Wall at {}", target);
      if (collect_annotations) {
        anns[target] = Ann::make_red();
        anns[coord] = Ann::make_magenta();
      }
      return false;
    }

    bool can_move{map[target] == WideCell::Empty};
    DDUMP(DEBUG_SIMULATION, can_move);

    if (!can_move) {
      DPRINT(DEBUG_SIMULATION, "=== Recursive {}", target);
      can_move = try_push_box_h(target, instr, update);
    }

    if (!can_move) {
      DPRINT(DEBUG_SIMULATION, "Can't move");
      anns[coord] = Ann::make_magenta();
      return false;
    }

    DPRINT(DEBUG_SIMULATION, "Moving from {} to {}", coord, target);

    update[coord] = WideCell::Empty;
    DPRINT(DEBUG_SIMULATION, "Clearing {}", coord);

    update[target] = map[coord];
    DPRINT(DEBUG_SIMULATION, "Moving '{}' to {}", map[coord].str(), target);

    // update[target.in_dir(instr)] = map[coord.in_dir(instr)];
    // DPRINT(DEBUG_SIMULATION, "Moving '{}' to {}", map[coord.in_dir(instr)].str(), target.in_dir(instr));
    return true;
  }

  bool try_push_box_v(Coord2D coord, Dir2D instr, WideRobotMap::slice_t &update) {
    DPRINT(DEBUG_SIMULATION, "V-move {} at {}", instr.str(), coord);
    const WideCell port_cell_value{instr == Dir2D::Up ? WideCell::BoxLeft : WideCell::BoxRight};
    const WideCell starboard_cell_value{port_cell_value == WideCell::BoxLeft ? WideCell::BoxRight : WideCell::BoxLeft};

    const Coord2D port_coord{map[coord] == port_cell_value ? coord : coord.in_dir(instr.left())};
    const Coord2D starboard_coord{map[coord] == starboard_cell_value ? coord : coord.in_dir(instr.right())};

    const Coord2D port_target{port_coord.in_dir(instr)};
    const Coord2D starboard_target{starboard_coord.in_dir(instr)};

    if (map[port_target] == WideCell::Wall || map[starboard_target] == WideCell::Wall) {
      if (collect_annotations && map[port_target] == WideCell::Wall) {
        anns[port_target] = Ann::make_red();
      }
      if (collect_annotations && map[starboard_target] == WideCell::Wall) {
        anns[starboard_target] = Ann::make_red();
      }
      if (collect_annotations) {
        anns[port_coord] = Ann::make_magenta();
        anns[starboard_coord] = Ann::make_magenta();
      }
      return false;
    }

    bool port_clear_other_branch{update.contains(port_target) && update[port_target] == WideCell::Empty};
    bool starboard_clear_other_branch{update.contains(starboard_target) && update[starboard_target] == WideCell::Empty};
    bool can_move_port{map[port_target] == WideCell::Empty};
    bool can_move_starboard{map[starboard_target] == WideCell::Empty};
    can_move_port = can_move_port || port_clear_other_branch;
    can_move_starboard = can_move_starboard || starboard_clear_other_branch;

    if (!can_move_port) {
      DDUMP(DEBUG_SIMULATION, map[port_target] == port_cell_value);
      if (map[port_target] == port_cell_value) {
        DPRINT(DEBUG_SIMULATION, "Before pushing straight V-move {} at {}", instr.str(), coord);
        can_move_starboard = can_move_port = try_push_box_v(port_target, instr, update);
        DPRINT(DEBUG_SIMULATION, "After pushing straight V-move {} at {}", instr.str(), coord);
        if constexpr (DEBUG & DEBUG_SIMULATION_PUSH_VERBOSE) { render_slice(update); };
      } else {
        DPRINT(DEBUG_SIMULATION, "Before pushing port-port V-move {} at {}", instr.str(), coord);
        can_move_port = try_push_box_v(port_target.in_dir(instr.left()), instr, update);
        DPRINT(DEBUG_SIMULATION, "After pushing port-port V-move {} at {}", instr.str(), coord);
        if constexpr (DEBUG & DEBUG_SIMULATION_PUSH_VERBOSE) { render_slice(update); };
      }
    }

    if (!can_move_port) {
      if (collect_annotations) {
        anns[starboard_coord] = Ann::make_magenta();
        anns[port_coord] = Ann::make_magenta();
      }
      return false;
    }

    if (!can_move_starboard) {
      DPRINT(DEBUG_SIMULATION, "Before pushing starboard V-move {} at {}", instr.str(), coord);
      can_move_starboard = try_push_box_v(starboard_target, instr, update);
      DPRINT(DEBUG_SIMULATION, "After pushing starboard V-move {} at {}", instr.str(), coord);
      if constexpr (DEBUG & DEBUG_SIMULATION_PUSH_VERBOSE) { render_slice(update); };
    }

    if (!can_move_starboard) {
      if (collect_annotations) {
        anns[starboard_coord] = Ann::make_magenta();
        anns[port_coord] = Ann::make_magenta();
      }
      return false;
    }

    update[port_coord] = WideCell::Empty;
    update[starboard_coord] = WideCell::Empty;
    update[port_target] = port_cell_value;
    update[starboard_target] = starboard_cell_value;
    DPRINT(DEBUG_SIMULATION, "Own updates of V-move {} at {}", instr.str(), coord);
    if constexpr (DEBUG & DEBUG_SIMULATION_PUSH_VERBOSE) { render_slice(update); };

    return true;
  }

  void simulate() {
    collect_annotations = collect_annotations || visualize;
    debug_if<DEBUG_SIMULATION>([&]() {
      map.render({});
    });

    auto bar = (!DEBUG && visualize) ? make_bar("Running instructions 🏃 ", instructions.size()) : nullptr;

    if (!DEBUG && visualize) {
      cls();
    }

    Coord2D tl{map.top_left_coord()}, br{map.bottom_right_coord()};
    if (collect_annotations) {
      for (int y = tl.y; y <= br.y; ++y) {
        for (int x = tl.x; x <= br.x; ++x) {
          if (map[{x, y}] == WideCell::Wall) {
            static_annotations[{x, y}] = Ann::make_bright_grey();
          }
          if (map[{x, y}] == WideCell::Empty) {
            static_annotations[{x, y}] = Ann::make_bright_grey();
          }
        }
      }
    }

    bool has_moved{true};
    Dir2D prev_instr{instructions[0].left()};

    int ip{0};
    for(auto instr: instructions) {
      DPRINT(DEBUG_SIMULATION_TOPLEVEL, "Commencing step {} {} (prev {}, moved {})", ip, instr.str(), prev_instr.str(), has_moved);

      if (!has_moved && instr == prev_instr) {
        DPRINT(DEBUG_SIMULATION_TOPLEVEL, "Not moved on previous step, instruction is the same - {}", instr.str());
        ++ip;
        continue;
      }

      if (collect_annotations) {
        anns = static_annotations;
      }

      if (ip == 1345) {
        map.render({});
      }

      prev_instr = instr;
      has_moved = simulation_step(instr);
      DPRINT(DEBUG_SIMULATION_TOPLEVEL, "{}", has_moved);

      if (visualize) {
        if constexpr (!DEBUG) {
          std::print("\033[1;1H");
          bar->tick();
          std::print("\033[3;1H\033[0m");
        }
        map.render(anns);
        std::println("Step: {}", ip);
        std::this_thread::sleep_for(visualisation_delay);
      }
      ++ip;
    }

    if (visualize && !DEBUG) {
      anns = static_annotations;
      std::print("\033[1;1H");
      bar.reset();
      std::print("\033[3;1H\033[0m");
      map.render(anns);
    }
  }
};

struct Map {
  struct Cell {
    enum class Value : uint8_t {
      Wall, Empty, Box,
    } m_val;

    Cell() = default;

    constexpr static Value Wall = Value::Wall;
    constexpr static Value Empty = Value::Empty;
    constexpr static Value Box = Value::Box;

    constexpr Cell(Value val) : m_val(val) {};
    constexpr operator Value() const { return m_val; }

    explicit operator bool() const = delete;

    std::string to_string() const {
      switch (m_val) {
      case Value::Wall: return "#";
      case Value::Empty: return ".";
      case Value::Box: return "O";
      }
    }
  };

  using MapContainer = std::map<Coord2D, Cell>;
  MapContainer map;
  Coord2D min_coord{INT_MAX, INT_MAX}, max_coord{INT_MIN, INT_MIN};

  template <bool> struct IteratorPointerType;
  template<> struct IteratorPointerType<false> {
    using value_type        = Cell;
    using pointer           = Cell*;
    using reference         = Cell&;
    using map_pointer_type  = Map*;
  };
  template<> struct IteratorPointerType<true> {
    using value_type        = const Cell;
    using pointer           = const Cell*;
    using reference         = const Cell&;
    using map_pointer_type  = const Map*;
  };

  template <bool is_const>
  class YFirstIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = int;
    using value_type = IteratorPointerType<is_const>::value_type;
    using pointer = IteratorPointerType<is_const>::pointer;
    using reference = IteratorPointerType<is_const>::reference;

    using map_pointer_type  = IteratorPointerType<is_const>::map_pointer_type;

    YFirstIterator(map_pointer_type map): m_map{map}, m_cur{map->min_coord.x, map->min_coord.y} {}
    YFirstIterator(map_pointer_type map, Coord2D cur): m_map{map}, m_cur{cur} {}

    bool is_start_of_line() const {
      return m_cur.x == m_map->min_coord.x;
    }

    reference operator*() const {
      if (m_map->map.contains(m_cur)) {
        return m_map->map[m_cur];
      }
      throw std::runtime_error(std::format("Index {} out of bounds {} - {}", m_cur, m_map->min_coord, m_map->max_coord));
    }

    pointer operator->() const {
      if (m_map->map.contains(m_cur)) {
        return &m_map->map.at(m_cur);
      }
      throw std::runtime_error(std::format("Index {} out of bounds {} - {}", m_cur, m_map->min_coord, m_map->max_coord));
    }

    YFirstIterator<is_const> operator++() {
      ++m_cur.x;
      if (m_cur.x > m_map->max_coord.x) {
        m_cur.x = m_map->min_coord.x;
        ++m_cur.y;
      }
      return *this;
    }

    YFirstIterator<is_const> operator++(int) {
      YFirstIterator<is_const> tmp{*this};
      ++(*this);
      return tmp;
    }

    friend bool operator==(const YFirstIterator<is_const> &a, const YFirstIterator<is_const> &b) {
      return a.m_cur == b.m_cur;
    }

    friend bool operator!=(const YFirstIterator<is_const> &a, const YFirstIterator<is_const> &b) {
      return a.m_cur != b.m_cur;
    }

    Coord2D coord() const { return m_cur; }

  private:
    map_pointer_type m_map;
    Coord2D m_cur;
  };

  using const_iterator = YFirstIterator<true>;
  using iterator = YFirstIterator<false>;

  iterator begin() { return iterator(this); }
  iterator end() { return iterator(this, {min_coord.x, max_coord.y + 1}); }

  const_iterator cbegin() const { return const_iterator(this);}
  const_iterator cend() const { return const_iterator(this, {min_coord.x, max_coord.y + 1});}

  Cell& operator[](Coord2D c) {
    return map.at(c);
  }

  static std::pair<Map, Coord2D> parse_map(std::istream &in) {
    std::string line;
    Map result{};
    Coord2D robot_pos;

    result.min_coord = {0, 0};
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
          result.map[{x, y}] = Cell::Empty;
          robot_pos = {x, y};
          break;
        default:
          throw std::runtime_error(std::format("Unknown char in map {} at {}", c, Coord2D{x, y}));
        }
        result.max_coord.x = std::max(result.max_coord.x, x);
        ++x;
      }
      result.max_coord.y = y;
      ++y;
    }
    return std::make_pair(result, robot_pos);
  }

  void render(Coord2D robot_coord, std::string robot_str) const {
    annotated_render([&](auto cur, auto cell) {
      if (cur == robot_coord) {
        return robot_str;
      } else {
        return std::string{cell};
      }
    });
    std::cout << "\n";
  }

  void annotated_render(std::function<std::string(Coord2D cur, std::string_view cell)> annotate_fn) const {
    for (auto it = cbegin(); it != cend(); ++it) {
      if (it.is_start_of_line() && it != cbegin()) {
        std::cout << "\n";
      }
      std::cout << annotate_fn(it.coord(), it->to_string());
    }
    std::cout << "\n";
  }

  static GPS gps(Coord2D c) {
    return c.y * 100 + c.x;
  }

  GPS gps_all_boxes() const {
    GPS result{};
    for (auto pair: map) {
      if (pair.second == Cell::Box) {
        result += gps(pair.first);
      }
    }
    return result;
  }
};

std::ostream& operator<<(std::ostream& os, Map::Cell c) {
  switch (c) {
  case Map::Cell::Empty: os << '.'; break;
  case Map::Cell::Wall: os << '#'; break;
  case Map::Cell::Box: os << 'O'; break;
  }
  return os;
}

std::vector<Dir2D> parse_instructions(std::istream& in) {
  std::string line;
  std::vector<Dir2D> result;
  while (std::getline(in, line)) {
    for (char c: line) {
      result.push_back(parse_direction(c));
    }
  }
  return result;
}

std::pair<Map, Coord2D> simulate(const Map &initial_map, Coord2D robot_coord, std::vector<Dir2D> instructions) {
  Map map{initial_map};

  for(auto instr: instructions) {
    Coord2D iteration_initial_cord{robot_coord};

    debug_if<DEBUG_SIMULATION>([]() {cls(); });

    const Coord2D target = robot_coord.in_dir(instr);

    if (map[target] == Map::Cell::Empty) {
      robot_coord = target;
      debug_if_slow<DEBUG_SIMULATION>(simulation_delay, [&]() {
        map.annotated_render([&](Coord2D c, std::string_view cell_str) {
          std::ostringstream os;
          os << termcolor::colorize;
          if ( c == iteration_initial_cord ) {
            os << termcolor::cyan << instr << termcolor::reset;
          } else if ( c == target ) {
            os << termcolor::green << "@" << termcolor::reset;
          } else {
            os << cell_str;
          }
          return os.str();
        });
        std::println("Found an empty cell to move in - {} - from {}", robot_coord, iteration_initial_cord);
      });
      continue;
    }

    if (map[target] == Map::Cell::Box) {
      Coord2D cur{target.in_dir(instr)};
      std::set<Coord2D> debug_search_span{};
      while (map[cur] == Map::Cell::Box) {
        debug_if<DEBUG_SIMULATION>([&]() { debug_search_span.insert(cur); });
        cur.move_in_dir(instr);
      }
      if (map[cur] == Map::Cell::Empty) {
        map[cur] = Map::Cell::Box;
        map[target] = Map::Cell::Empty;
        robot_coord = target;
        debug_if_slow<DEBUG_SIMULATION>(simulation_delay, [&]() {
          map.annotated_render([&](Coord2D c, std::string_view cell_str) {
            std::ostringstream os;
            os << termcolor::colorize;
            if (debug_search_span.contains(c)) {
              os << termcolor::yellow << cell_str << termcolor::reset;
            } else if (c == iteration_initial_cord) {
              os << termcolor::cyan << instr << termcolor::reset;
            } else if (c == robot_coord) {
              os << termcolor::green << "@" << termcolor::reset;
            } else {
              os << cell_str;
            }
            return os.str();
          });
          std::println("Pushed some blocks and moved to {} from {}", robot_coord, iteration_initial_cord);
        });
        continue;
      }
    }

    debug_if_slow<DEBUG_SIMULATION>(simulation_delay, [&]() {
      map.annotated_render([&](Coord2D c, std::string_view cell_str) {
        std::ostringstream os;
        os << termcolor::colorize;
        if (c == iteration_initial_cord) {
          os << termcolor::red << "@" << termcolor::reset;
        } else if (c == target) {
          os << termcolor::red << cell_str << termcolor::reset;
        } else {
          os << cell_str;
        }
        return os.str();
      });
      std::println("Blocked at {}", robot_coord);
    });
  }
  return std::make_pair(map, robot_coord);
}


int main(int argc, char **argv) {
  std::vector<std::string> map_input{};
  std::string line;

  while (getline(std::cin, line)) {
    if (line == "") {
      break;
    }
    map_input.push_back(line);
  }
  auto instructions = parse_instructions(std::cin);

  auto map = mk_map<WideRobotMap>(map_input);

  auto simulation = WideSimulator{
    .map = map,
    .instructions = instructions,
    .visualize = false,
    .collect_annotations = true,
    .visualisation_delay = 50ms,
  };
  simulation.map.render({});
  std::println();
  simulation.simulate();
  simulation.map.render({});
  std::println("GPS cost {}", simulation.map.gps_all_boxes());

  // auto [map, robot_coord] = Map::parse_map(std::cin);
  // auto instructions = parse_instructions(std::cin);
  // std::println("Instructions of size {}", instructions.size());
  // dump(instructions);
  // std::println();
  // std::println("Parsed map of size {} - {}", map.min_coord, map.max_coord);
  // map.render(robot_coord, "@");

  // auto result = simulate(map, robot_coord, instructions);

  // std::println("\nGPS after simulation is complete: {}", result.first.gps_all_boxes());
  // result.first.render(result.second, "@");
}
