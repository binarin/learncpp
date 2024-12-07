#ifndef INDICATORS_DISPLAY_WIDTH
#define INDICATORS_DISPLAY_WIDTH


#include <clocale>
#include <cstdlib>
#include <memory>
#include <string>



namespace unicode {
  static inline int display_width(const std::string &input) {

    auto locale_restore = [](char *maybe_old) {
      setlocale(LC_ALL, maybe_old ? maybe_old : "");
    };

    using locale_scope_t = std::unique_ptr<char, decltype(locale_restore)>;
    locale_scope_t locale_guard(setlocale(LC_ALL, ""), locale_restore);

    auto wide_buf = std::make_unique_for_overwrite<wchar_t[]>(input.size() + 1);
    auto converted_len = std::mbstowcs(&wide_buf[0], input.c_str(), input.size());
    if (converted_len < 0) {
      // invalid input, let's fallback to input length
      return input.size();
    }
    return wcswidth(&wide_buf[0], converted_len);
  }

  static inline int display_width(const std::wstring &input) {
    return wcswidth(input.c_str(), input.size());
  }

} // namespace unicode

#endif
