#include "helpers.hpp"

bool is_valid_arithmetic_expression(const std::string &s) {
  // If the input contains only numbers, (), [], {}, +-*/^ and all brackets are
  // correctly opened and closed, automatically eval This is to make it easier
  // for the user to use the program

  // If the string is empty or all spaces, return false
  if (s.empty() || s.find_first_not_of(' ') == std::string::npos) {
    return false;
  }

  std::string allowed = "0123456789.+-*/^()[]{} ";
  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == '(') {
      // Check that the bracket has a valid closing bracket
      size_t closing_bracket =
          eval_with_steps::get_corresponding_closing_bracket(s.c_str(), i);
      if (closing_bracket == std::string::npos) {
        return false;
      }
    }
    if (s[i] == ')') {
      // Check that the bracket has a valid opening bracket
      size_t opening_bracket =
          eval_with_steps::get_back_corresponding_bracket(s.c_str(), i);
      if (opening_bracket == std::string::npos) {
        return false;
      }
    }

    if (allowed.find(s[i]) == std::string::npos) {
      return false;
    }
  }
  return true;
}