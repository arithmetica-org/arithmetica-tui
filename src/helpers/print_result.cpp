#include "helpers.hpp"

void print_result(std::string str, std::ostream &outstream) {
  auto v = get_printable_result(str);
  outstream << v[0] << "\n" << v[1] << "\n" << v[2] << "\n";
}