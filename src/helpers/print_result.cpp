#include "helpers.hpp"

void print_result(std::string str) {
  auto v = get_printable_result(str);
  std::cout << v[0] << "\n" << v[1] << "\n" << v[2] << "\n";
}