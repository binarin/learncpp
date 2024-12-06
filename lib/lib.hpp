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

class NonCopyable
{
private:
    auto operator=( NonCopyable const& ) -> NonCopyable& = delete;
    NonCopyable( NonCopyable const& ) = delete;
public:
    auto operator=( NonCopyable&& ) -> NonCopyable& = default;
    NonCopyable() = default;
    NonCopyable( NonCopyable&& ) = default;
};

class ScopeGuard : public NonCopyable {
private:
  std::function<void()>    cleanup_;
public:
  friend
  void dismiss( ScopeGuard& g ) { g.cleanup_ = []{}; }

  ~ScopeGuard() { cleanup_(); }

  template< class Func >
  ScopeGuard( Func const& cleanup )
    : cleanup_( cleanup )
  {}

  ScopeGuard( ScopeGuard&& other )
    : cleanup_( std::move( other.cleanup_ ) )
  { dismiss( other ); }
};

#endif
