#include "helpers.hpp"

std::string center(std::string str, size_t n) {
  return std::string((n - str.length()) / 2, ' ') + str;
}