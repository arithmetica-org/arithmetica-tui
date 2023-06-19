#include "helpers.hpp"

std::string remove_extra_front_back_brackets(std::string str) {
  while (str.length() >= 2 &&
         get_matching_brace(str.c_str(), 0) == str.length() - 1) {
    str = str.substr(1, str.length() - 2);
  }
  return str;
}