#include "helpers.hpp"

int compare_printed_text(const std::string &a, const std::string &b) {
  bool same_text = compare_text_without_ansi(a, b);
  if (same_text && a.length() > b.length()) {
    return 1;
  } else if (same_text && b.length() >= a.length()) {
    return 2;
  } else {
    return -1;
  }
}