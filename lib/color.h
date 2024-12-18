#pragma once

#include "indicators/termcolor.hpp"
#include <sstream>

template <typename Mod>
std::string colored(Mod mod, std::string_view s) {
  std::ostringstream os;
  os << termcolor::colorize << mod << s << termcolor::reset;
  return os.str();
}
