#include "helpers.hpp"

size_t get_matching_brace(std::string str, size_t index) {
  if (str[index] != '(')
    return -1;
  int count = 0;
  for (size_t i = index; i < str.length(); i++) {
    if (str[i] == '(')
      count++;
    if (str[i] == ')')
      count--;

    if (!count)
      return i;
  }
  return -1;
}