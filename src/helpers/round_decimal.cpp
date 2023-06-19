#include "helpers.hpp"

std::string round_decimal(std::string decimal, int n) {
  // Find the position of the decimal point
  size_t decimal_pos = decimal.find('.');
  if (decimal_pos == std::string::npos) {
    // The decimal has no decimal point
    return decimal;
  }
  // Find the number of decimal places to keep
  size_t end_pos = decimal_pos + n + 1;
  if (end_pos >= decimal.size()) {
    // The decimal has fewer than n decimal places
    return decimal;
  }
  // Round the decimal to n places
  int round_digit = decimal[end_pos] - '0';
  if (round_digit >= 5) {
    // Round up the last decimal place
    int carry = 1;
    for (int i = end_pos - 1; i >= 0; i--) {
      if (decimal[i] == '.') {
        continue;
      }
      int digit = decimal[i] - '0' + carry;
      carry = digit / 10;
      digit %= 10;
      decimal[i] = digit + '0';
      if (carry == 0) {
        break;
      }
    }
    if (carry != 0) {
      // Insert a new digit before the decimal point
      decimal.insert(decimal.begin() + decimal_pos, carry + '0');
      decimal_pos++;
    }
  }
  // Truncate the decimal to n places
  return basic_math_operations::add(decimal.substr(0, end_pos), "0");
}