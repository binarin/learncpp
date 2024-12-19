#pragma once

#include "coord.h"

#include <map>
#include <utility>

namespace grid {

  template <class Cell>
  class Grid {
  public:
    using value_t = Cell;

    Grid(Grid &&other)
    : m_top_left{other.m_top_left},
      m_bottom_right{other.m_bottom_right},
      m_map{std::move(other.m_map)}
    {}

    Grid(const Grid& other) 
    : m_top_left{other.m_top_left},
      m_bottom_right{other.m_bottom_right},
      m_map{other.m_map}
    {}

    Grid(int width, int height)
    : m_top_left{0, 0}, m_bottom_right{width - 1, height - 1} {}

    Grid() : m_top_left{0, 0}, m_bottom_right{-1, -1}, m_map{} {}

    void set(Coord2D c, Cell val);
    Cell operator[] (Coord2D c) const;

    bool is_within_bounds(Coord2D c) const;

    std::pair<Coord2D, Coord2D> bounds() const { return std::make_pair(m_top_left, m_bottom_right); }

  private:
    Coord2D m_top_left{0, 0};
    Coord2D m_bottom_right{-1, -1};
    std::map<Coord2D, Cell> m_map{};
  };

  template <class Cell>
  void Grid<Cell>::set(Coord2D c, Cell val) {
    if (!is_within_bounds(c)) {
      throw std::runtime_error("Can't grow");
    }
    m_map[c] = val;
  }

  template <class Cell>
  Cell Grid<Cell>::operator[](Coord2D c) const {
    if (m_map.contains(c)) {
      return m_map.at(c);
    }
    if (is_within_bounds(c)) {
      return Cell{};
    }
    return Cell::out_of_bounds();
  }

  template <class Cell>
  bool Grid<Cell>::is_within_bounds(Coord2D c) const {
    return c.x >= m_top_left.x && c.x <= m_bottom_right.x &&
           c.y >= m_top_left.y && c.y <= m_bottom_right.y;
  }

}
