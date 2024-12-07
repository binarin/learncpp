#include "lib.hpp"
#include <memory>

using namespace indicators;

std::unique_ptr<indicators::BlockProgressBar, std::function<void(indicators::BlockProgressBar*)>> make_bar(const std::string &prefix, int max_progress) {
  BlockProgressBar *bar = new BlockProgressBar {
    option::PrefixText{prefix},
    option::BarWidth{60},
    option::ForegroundColor{Color::yellow},
    option::ShowElapsedTime{true},
    option::ShowRemainingTime{true},
    option::MaxProgress{max_progress},
  };

  auto finalize_bar = [](BlockProgressBar *bar) {
    bar->mark_as_completed();
    show_console_cursor(true);
  };

  std::unique_ptr<indicators::BlockProgressBar,
                  std::function<void(indicators::BlockProgressBar *)>>
    result(bar, finalize_bar);

  show_console_cursor(false);

  return result;
}
