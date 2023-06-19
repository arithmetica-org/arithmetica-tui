#include "helpers.hpp"

bool compare_text_without_ansi(std::string a, std::string b) {
  a += " ";
  b += " "; // Add a space to the end of each string in case they end with an
            // ANSI escape sequence

  std::string::size_type pos_a = 0;
  std::string::size_type pos_b = 0;
  while (pos_a < a.length() && pos_b < b.length()) {
    if (a[pos_a] == '\033') {
      // Skip ANSI escape sequence in string a
      while (pos_a < a.length() && a[pos_a] != 'm') {
        pos_a++;
      }
      // Move past 'm' character
      pos_a++;
    } else if (b[pos_b] == '\033') {
      // Skip ANSI escape sequence in string b
      while (pos_b < b.length() && b[pos_b] != 'm') {
        pos_b++;
      }
      // Move past 'm' character
      pos_b++;
    }
    // Compare characters
    if (a[pos_a] != b[pos_b]) {
      return false;
    }
    pos_a++;
    pos_b++;
  }
  return pos_a >= a.length() && pos_b >= b.length();
}