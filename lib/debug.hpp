#pragma once

#define DEBUG_BIT(NAME, BIT) [[maybe_unused]] constexpr uint32_t NAME{1 << BIT};

#define SETUP_DEBUG(val)                                                                                       \
  constexpr uint32_t DEBUG = val;                                                                              \
                                                                                                               \
  template <uint32_t flags, typename T> void debug_if(T fn) {                                                  \
    if constexpr (DEBUG & flags) {                                                                             \
      fn();                                                                                                    \
    }                                                                                                          \
  }                                                                                                            \
                                                                                                               \
  template<uint32_t flags, class... Args>                                                                      \
  void println_if(std::format_string<Args...> fmt, Args&&... args) {                                           \
    if constexpr (DEBUG & flags) {                                                                             \
      std::cout << std::vformat(fmt.get(), std::make_format_args(args...)) << "\n";                            \
    }                                                                                                          \
  }                                                                                                            \
                                                                                                               \
  template<uint32_t flags, typename T> void debug_if_slow(int millis, T fn) {                                  \
    if constexpr (DEBUG & flags) {                                                                             \
      fn();                                                                                                    \
      std::this_thread::sleep_for(std::chrono::milliseconds(millis));                                          \
    }                                                                                                          \
  }




#define DDUMP(flags, expr) println_if<flags>("{}({}():{}) = {}", #expr, __FUNCTION__, __LINE__, (expr))
