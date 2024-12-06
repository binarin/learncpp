#ifndef __LIB_HPP__
#define __LIB_HPP__

#include <algorithm>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <ranges>
#include <set>
#include <algorithm>
#include <iostream>
#include <iterator>

template <class Container, typename T = typename Container::value_type> void dump(const Container &cont) {
  std::copy(cont.begin(), cont.end(), std::ostream_iterator<T>(std::cout, " "));
}

#endif
