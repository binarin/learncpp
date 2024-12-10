#pragma once

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
};
