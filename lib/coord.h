#pragma once
#include <algorithm>
#include <format>
#include <sstream>

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
