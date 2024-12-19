#pragma once
#include <algorithm>
#include <cstdint>
#include <format>
#include <sstream>
#include <array>

struct Dir2D {
  enum Value : uint_fast8_t  {
    Up, Right, Down, Left
  } m_val;

  Dir2D(const Dir2D &) = default;
  Dir2D(Dir2D &&) = default;
  Dir2D &operator=(const Dir2D &) = default;
  Dir2D &operator=(Dir2D &&) = default;

  // Dir2D(const Dir2D &o) : m_val{o.m_val} {}
  // Dir2D(Dir2D &&o) : m_val{o.m_val} {};
  // Dir2D &operator=(const Dir2D &o) { m_val = o.m_val; return *this; }
  // Dir2D &operator=(Dir2D &&o) { m_val = o.m_val; return *this; }

  Dir2D(Value val) : m_val(val) {};

  static constexpr std::array<Dir2D, 4> all();

  int idx() const {
    switch (m_val) {
    case Up: return 0;
    case Right: return 1;
    case Down: return 2;
    case Left: return 3;
    }
  }

  bool operator==(Value val) const { return m_val == val; }
  operator int() const { return static_cast<int>(m_val); }

  Dir2D reverse() const {
    switch (m_val) {
    case Up: return Down;
    case Right: return Left;
    case Down: return Up;
    case Left: return Right;
    }
  }

  Dir2D left() const {
    switch (m_val) {
    case Up: return Left;
    case Right: return Up;
    case Down: return Right;
    case Left: return Down;
    }
  }

  Dir2D right() const {
    switch (m_val) {
    case Up: return Right;
    case Right: return Down;
    case Down: return Left;
    case Left: return Up;
    }
  }

  std::string str() const {
    switch (m_val) {
    case Up: return "↑";
    case Right: return "→";
    case Down: return "↓";
    case Left: return "←";
    }
  }
};

inline constexpr std::array<Dir2D, 4> Dir2D::all() {
  return {Up, Right, Down, Left};
};


inline std::ostream& operator<<(std::ostream& os, Dir2D dir) {
  switch (dir.m_val) {
  case Dir2D::Up: os << "↑"; break;
  case Dir2D::Right: os << "→"; break;
  case Dir2D::Down: os << "↓"; break;
  case Dir2D::Left: os << "←"; break;
  }
  return os;
}


struct Coord2D {
  int x = 0;
  int y = 0;
  inline bool operator<(const Coord2D &other) const {
    return (x < other.x) || ((x == other.x) && (y < other.y));
  }
  inline bool operator==(const Coord2D &other) const {
    return (x == other.x) && (y == other.y);
  }
  inline Coord2D operator+(const Coord2D &other) const {
    return {x + other.x, y + other.y};
  }
  inline Coord2D operator-(const Coord2D &other) const {
    return {x - other.x, y - other.y};
  }
  inline Coord2D &operator+=(const Coord2D &other) {
    x += other.x;
    y += other.y;
    return *this;
  }
  inline Coord2D &operator-=(const Coord2D &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  inline void move_in_dir(Dir2D dir) {
    Coord2D tmp{in_dir(dir)};
    x = tmp.x;
    y = tmp.y;
  }

  inline Coord2D in_dir(Dir2D dir) const {
    switch (dir.m_val) {
    case Dir2D::Up: return {x, y - 1};
    case Dir2D::Right: return {x + 1, y};
    case Dir2D::Down: return {x, y + 1};
    case Dir2D::Left: return {x - 1, y};
    }
  }

  inline Coord2D up() const {
    return {x, y - 1};
  };
  inline Coord2D down() const {
    return {x, y + 1};
  };
  inline Coord2D left() const {
    return {x - 1, y};
  };
  inline Coord2D right() const {
    return {x + 1, y};
  };

  inline bool within_bounds(Coord2D top_left, Coord2D bottom_right) {
    return x >= top_left.x && x <= bottom_right.x && y >= top_left.y && y <= bottom_right.y;
  }

  void maybe_update_lower_boundary(const Coord2D& coord) {
    x = std::min(x, coord.x);
    y = std::min(y, coord.y);
  }

  void maybe_update_upper_boundary(const Coord2D& coord) {
    x = std::max(x, coord.x);
    y = std::max(y, coord.y);
  }
};


template<>
struct std::formatter<Coord2D, char> {

  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext& ctx) {
    auto it = ctx.begin();
    if (it != ctx.end() && *it != '}') {
      throw std::format_error("Coord2D format doesn't support args");
    }
    return it;
  }

  template <class FmtContext>
  FmtContext::iterator format(const Coord2D& coord, FmtContext& ctx) const {
    std::ostringstream out;
    out << "(" << coord.x << ", " << coord.y << ")";
    return std::ranges::copy(std::move(out).str(), ctx.out()).out;
  }
};
