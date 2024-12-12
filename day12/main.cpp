#include "lib/coord.h"

#include "indicators/termcolor.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <format>
#include <iostream>
#include <list>
#include <map>
#include <ostream>
#include <print>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Region;

struct Cell {
  Cell(Region* region, Coord2D coord): region(region), coord(coord) {}
  Region* region;
  bool border_left, border_right, border_down, border_up;
  std::optional<int> fence_id_left, fence_id_right, fence_id_down, fence_id_up;
  Coord2D coord;

  int num_borders() const {
    return border_up + border_down + border_left + border_right;
  }
};

struct Region {
  typedef int Crop;
  typedef int Id;

  Crop crop;
  Id id;
  std::list<Cell*> cells;

  Region(Id id, Crop crop) : crop(crop), id(id) {}
};

struct GardenMap {
  typedef decltype(&termcolor::red) ColorMod;
  typedef std::vector<ColorMod> Colors;
  const static Colors colors;


  Region::Id last_allocated_region;
  Coord2D minCoord{INT_MAX, INT_MAX};
  Coord2D maxCoord{INT_MIN, INT_MIN};
  std::map<Coord2D, Cell> cells;
  std::map<Region::Id, Region> regions;

  void add_single_cell_region(Region::Crop crop, Coord2D coord) {
    if (cells.contains(coord)) {
      throw new std::logic_error(
          std::format("Cell already defined at {}", coord));
    }

    ++last_allocated_region;
    auto [regionKV, inserted] = regions.try_emplace(last_allocated_region, last_allocated_region, crop);
    assert(inserted);

    Region &new_region = regionKV->second;

    Cell &new_cell = cells.try_emplace(coord, &new_region, coord).first->second;
    new_region.cells.push_back(&new_cell);

    minCoord.maybe_update_lower_boundary(coord);
    maxCoord.maybe_update_upper_boundary(coord);
  }

  std::optional<std::string> validate() const {
    for (int y = minCoord.y; y <= maxCoord.y; y++) {
      for (int x = minCoord.x; x <= maxCoord.x; x++) {
        if (!cells.contains({x, y})) {
          return std::format("Cell at {} is not initialized", Coord2D{x, y});
        }
      }
    }
    return {};
  }

  [[maybe_unused]] static constexpr int CROSSING_UP_BIT = 0;
  [[maybe_unused]] static constexpr int CROSSING_RIGHT_BIT = 1;
  [[maybe_unused]] static constexpr int CROSSING_DOWN_BIT = 2;
  [[maybe_unused]] static constexpr int CROSSING_LEFT_BIT = 3;
  [[maybe_unused]] static constexpr int CROSSING_VERTICAL = (1 << CROSSING_UP_BIT) | (1 << CROSSING_DOWN_BIT);
  [[maybe_unused]] static constexpr int CROSSING_HORIZONTAL = (1 << CROSSING_LEFT_BIT) | (1 << CROSSING_RIGHT_BIT);

  static constexpr std::array<std::string_view, 16> fence_chars{
    " ", // 0000
    "╵", // 0001
    "╶", // 0010
    "└", // 0011
    "╷", // 0100
    "│", // 0101
    "┌", // 0110
    "├", // 0111
    "╴", // 1000
    "┘", // 1001
    "─", // 1010
    "┴", // 1011
    "┐", // 1100
    "┤", // 1101
    "┬", // 1110
    "┼", // 1111
  };

  std::string_view fence_crossing(Coord2D coord) const {
    auto [x, y] = coord;
    uint8_t fence_mask{0};

    bool xy_has_up{}, xy_has_left{}, above_cell_has_left{}, left_cell_has_up{};

    if (cells.contains(coord)) {
      xy_has_up = cells.at({x, y}).border_up;
      xy_has_left = cells.at({x, y}).border_left;
    } else {
      if (cells.contains({x, y - 1})) {
        xy_has_up = cells.at({x, y -1}).border_down;
      }
      if (cells.contains({x - 1, y})) {
        xy_has_left = cells.at({x - 1, y}).border_right;
      }
    }

    if (cells.contains({x, y - 1})) {
      above_cell_has_left = cells.contains({x, y - 1}) && cells.at({x, y - 1}).border_left;
    } else if (cells.contains({x - 1, y - 1})) {
      above_cell_has_left = cells.at({x - 1, y - 1}).border_right;
    }

    if (cells.contains({x - 1, y})) {
      left_cell_has_up = cells.contains({x - 1, y}) && cells.at({x - 1, y}).border_up;
    } else if (cells.contains({x - 1, y - 1})) {
      left_cell_has_up = cells.at({x - 1, y - 1}).border_down;
    }

    fence_mask |= above_cell_has_left << CROSSING_UP_BIT;
    fence_mask |= xy_has_up << CROSSING_RIGHT_BIT;
    fence_mask |= xy_has_left << CROSSING_DOWN_BIT;
    fence_mask |= left_cell_has_up << CROSSING_LEFT_BIT;

    return fence_chars[fence_mask];
  }

  void render() const {
    for (int yUnsafe = minCoord.y; yUnsafe <= maxCoord.y + 1; yUnsafe++) { // intentionally goes out of bound due to '+1'
      std::string upper_fence_render{};
      std::ostringstream cells_render_stream;
      cells_render_stream << termcolor::colorize;
      for (int x = minCoord.x; x <= maxCoord.x; x++) {
        /*
          +- → uppder_fence_render(repeated)
          |c → cells_render (repeated)

         */
        if (yUnsafe != maxCoord.y + 1) {
          auto &cell = cells.at({x, yUnsafe});

          upper_fence_render.append(fence_crossing({x, yUnsafe}));
          if (cell.border_up && cell.fence_id_up) {
            upper_fence_render.append("━");
          } else {
           upper_fence_render.append(cell.border_up ? fence_chars[CROSSING_HORIZONTAL] : fence_chars[0]);
          }

          cells_render_stream << fence_chars[cell.border_left ? CROSSING_VERTICAL : 0];
          cells_render_stream << colors[cell.region->id % colors.size()];
          cells_render_stream << static_cast<char>(cell.region->crop);
          cells_render_stream << termcolor::reset;
        } else {
          upper_fence_render.append(fence_crossing({x, yUnsafe}));
          upper_fence_render.append(fence_chars[CROSSING_HORIZONTAL]);
        }
      }
      std::cout << upper_fence_render << fence_crossing({maxCoord.x + 1, yUnsafe}) << "\n";
      if (cells_render_stream.tellp()) {
        cells_render_stream << fence_chars[CROSSING_VERTICAL];
      }
      std::cout << cells_render_stream.str() << "\n";
    }
    std::cout << std::endl;
  }

  void place_fences() {

    for (int y = minCoord.y; y <= maxCoord.y; y++) {
      cells.at({minCoord.x, y}).border_left = true;
      for (int x = minCoord.x; x < maxCoord.x; x++) {
        if (cells.at({x, y}).region->crop != cells.at({x+1, y}).region->crop) {
          cells.at({x, y}).border_right = true;
          cells.at({x+1, y}).border_left = true;
        }
      }
      cells.at({maxCoord.x, y}).border_right = true;
    }

    for (int x = minCoord.x; x <= maxCoord.x; x++) {
      cells.at({x, minCoord.y}).border_up = true;
      for (int y = minCoord.y; y < maxCoord.y; y++) {
        if (cells.at({x, y}).region->crop != cells.at({x, y+1}).region->crop) {
          cells.at({x, y}).border_down = true;
          cells.at({x, y+1}).border_up = true;
        }
      }
      cells.at({x, maxCoord.y}).border_down = true;
    }

    int fence_id_counter{};
    for (int y = minCoord.y; y <= maxCoord.y; y++) {
      std::optional<int> prev_up{}, prev_down{};
      std::optional<Region::Crop> prev_crop{};
      for (int x = minCoord.x; x <= maxCoord.x; x++) {
        auto &cell = cells.at({x, y});

        bool same_crop = prev_crop.has_value() && prev_crop.value() == cell.region->crop;

        if (cell.border_up) {
          if (prev_up && same_crop) {
            cell.fence_id_up = prev_up;
          } else {
            cell.fence_id_up = prev_up = std::make_optional(++fence_id_counter);
          }
        } else {
          prev_up = {};
        }
        if (cell.border_down) {
          if (prev_down && same_crop) {
            cell.fence_id_down = prev_down;
          } else {
            cell.fence_id_down = prev_down = std::make_optional(++fence_id_counter);
          }
        } else {
          prev_down = {};
        }
        prev_crop = std::make_optional(cell.region->crop);
      }
    }

    for (int x = minCoord.x; x <= maxCoord.x; x++) {
      std::optional<int> prev_left{}, prev_right{};
      std::optional<Region::Crop> prev_crop{};
      for (int y = minCoord.y; y <= maxCoord.y; y++) {
        auto &cell = cells.at({x, y});

        bool same_crop = prev_crop.has_value() && prev_crop.value() == cell.region->crop;

        if (cell.border_left) {
          if (prev_left && same_crop) {
            cell.fence_id_left = prev_left;
          } else {
            cell.fence_id_left = prev_left = std::make_optional(++fence_id_counter);
          }
        } else {
          prev_left = {};
        }
        if (cell.border_right) {
          if (prev_right && same_crop) {
            cell.fence_id_right = prev_right;
          } else {
            cell.fence_id_right = prev_right = std::make_optional(++fence_id_counter);
          }
        } else {
          prev_right = {};
        }
        prev_crop = std::make_optional(cell.region->crop);
      }
    }
  }

  void join_regions() {
    std::set<Coord2D> global_queue{};

    for(auto coord : std::ranges::views::keys(cells)) {
      global_queue.insert(coord);
    }

    while (!global_queue.empty()) {
      Coord2D coord = *global_queue.begin();
      global_queue.erase(global_queue.begin());

      Region* region = cells.at(coord).region;

      std::vector<Coord2D> local_queue{coord.up(), coord.down(), coord.left(), coord.right()};
      std::set<Coord2D> visited{};

      while (!local_queue.empty()) {
        Coord2D coord = local_queue.back();
        local_queue.pop_back();

        if (!cells.contains(coord)) {
          continue;
        }

        if (visited.contains(coord)) {
          continue;
        }
        visited.insert(coord);

        Cell *cell = &cells.at(coord);
        Region *other_region = cell->region;

        if (other_region->id == region->id) {
          continue;
        }

        if (other_region->crop != region->crop) {
          continue;
        }

        local_queue.push_back(coord.up());
        local_queue.push_back(coord.down());
        local_queue.push_back(coord.left());
        local_queue.push_back(coord.right());

        // update region in every cell
        std::ranges::for_each(other_region->cells, [&](auto &cell) {
          cell->region = region;
        });

        // steal cell pointers
        region->cells.splice(region->cells.end(), other_region->cells);
        assert(other_region->cells.empty());
      }
    }

    std::erase_if(regions, [](const auto& item) {
      return item.second.cells.empty();
    });

    last_allocated_region = 0;
    for(auto &region : std::views::values(regions)) {
      region.id = ++last_allocated_region;
    }
  }
};

const GardenMap::Colors GardenMap::colors = {
    &termcolor::reset,
    &termcolor::red,
    &termcolor::blue,
    &termcolor::green,
    &termcolor::cyan,
    &termcolor::magenta,
    &termcolor::bright_grey,
    &termcolor::yellow,
};

int main(int argc, char **argv) {
  std::string line;
  GardenMap map{};
  {
    int y{0};
    while (std::getline(std::cin, line)) {
      int x{0};
      std::ranges::for_each(line, [&](char c) { map.add_single_cell_region(c, {x++, y}); });
      ++y;
    }
  }
  if (auto err = map.validate()) {
    throw std::logic_error(err.value());
  }
  map.place_fences();
  map.join_regions();
  map.render();

  int64_t total{}, total2{};
  for (Region &region : std::views::values(map.regions)) {
    int64_t area{};
    int64_t fence_sections{};
    std::set<int> fence_spans{};
    auto insert_span = [&](int span) {
      fence_spans.insert(span);
      return std::optional<int>{};
    };

    for (Cell *cell: region.cells) {
      ++area;
      fence_sections += cell->num_borders();
      cell->fence_id_down.and_then(insert_span);
      cell->fence_id_left.and_then(insert_span);
      cell->fence_id_right.and_then(insert_span);
      cell->fence_id_up.and_then(insert_span);
    }

    int64_t cost{fence_sections * area};
    int64_t cost2{static_cast<int64_t>(fence_spans.size() * area)};
    //std::println("Region with crop {} has price {} * {} = {}", static_cast<char>(region.crop), area, fence_sections, cost);
    std::println("Region with crop {} has span price {} * {} = {}", static_cast<char>(region.crop), area, fence_spans.size(), cost2);

    for (auto fence_id : fence_spans) {
      std::cout << fence_id << " ";
    }
    std::cout << "\n";
    total += cost;
    total2 += cost2;
  }
  std::println("Total cost {}", total);
  std::println("Total span cost {}", total2);

}
