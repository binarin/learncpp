#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <ranges>
/*
XMAS X  X
....  MM
....  AA
.... S  S
*/
int main(int argc, char **argv) {
  std::vector<std::string> input{};
  std::string line;
  while (std::getline(std::cin, line)) {
    input.push_back(line);
  };
  int height = input.size();
  int width = input[0].size();

  int result{0};

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if ( x < width - 3 ) {
        std::string hor{input[y][x + 0], input[y][x + 1],
                        input[y][x + 2], input[y][x + 3]};
        if (hor == "XMAS" || hor == "SAMX") {
          // std::cout << "hor " << y << " " << x << " " << hor << std::endl;
          ++result;
        }
      }

      if ( y < height - 3 ) {
        std::string ver{input[y + 0][x], input[y + 1][x],
                        input[y + 2][x], input[y + 3][x]};
        if (ver == "XMAS" || ver == "SAMX") {
          // std::cout << "ver " << y << " " << x << " " << ver << std::endl;
          ++result;
        }
      }

      if ( (y < height - 3) && (x < width - 3) ) {
        std::string dia1{""}, dia2{""};
        for (int ddia = 0; ddia < 4; ddia++) {
          dia1.push_back(input[y + ddia][x + ddia]);
          dia2.push_back(input[y + ddia][x + 3 - ddia]);
        }
        if (dia1 == "XMAS" || dia1 == "SAMX") {
          // std::cout << "diaf " << x << " " << y << std::endl;
          result++;
        }
        if (dia2 == "XMAS" || dia2 == "SAMX") {
          // std::cout << "diar " << x + 3 << " " << y << std::endl;
          result++;
        }
      }
    }
  }
  std::cout << result << std::endl;

  result = 0;
  for (int y = 0; y < height - 2; y++) {
    for (int x = 0; x < width - 2; x++) {

      std::string dia1{""}, dia2{""};
      for (int ddia = 0; ddia < 3; ddia++) {
        dia1.push_back(input[y + ddia][x + ddia]);
        dia2.push_back(input[y + ddia][x + 2 - ddia]);
      }
      if ((dia1 == "MAS" || dia1 == "SAM") && (dia2 == "MAS" || dia2 == "SAM")) {
        std::cout << "X " << x + 1 << " " << y + 1 << std::endl;
        result++;
      }
    }
  }
  std::cout << result << std::endl;

}
