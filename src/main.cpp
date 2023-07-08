#include <iostream>

int arithmetica_tui(int argc, char **argv, std::istream &in = std::cin,
                    std::ostream &out = std::cout);

int main(int argc, char **argv) {
#ifdef _WIN32
  std::system("chcp 65001 > nul");
#endif
  return arithmetica_tui(argc, argv, std::cin, std::cout);
}