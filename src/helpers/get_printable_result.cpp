#include "helpers.hpp"

std::vector<std::string> get_printable_result(std::string str) {
  std::string whole, numerator, denominator;

  bool include_brackets = remove_extra_front_back_brackets(str) != str;
  str = remove_extra_front_back_brackets(str);

  // Find the first '/' that isn't in brackets
  int bracket_count = 0;
  int div_loc = -1;
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '(' && eval_with_steps::get_corresponding_closing_bracket(
                             str.c_str(), i) != -1) {
      bracket_count++;
    } else if (str[i] == ')' && eval_with_steps::get_back_corresponding_bracket(
                                    str.c_str(), i) != -1) {
      bracket_count--;
    } else if (str[i] == '/' && bracket_count == 0) {
      div_loc = i;
      break;
    }
  }

  if (div_loc != -1) {
    if (str.find(' ') == std::string::npos) {
      numerator = str.substr(0, div_loc);
      denominator = str.substr(div_loc + 1, str.length());
    } else {
      whole = str.substr(0, str.find(' '));
      numerator = str.substr(str.find(' ') + 3, div_loc - str.find(' ') - 3);
      denominator = str.substr(div_loc + 1, str.length());
    }
  } else {
    return {std::string(str.length() + include_brackets * 2, ' '),
            (include_brackets ? "(" + str + ")" : str),
            std::string(str.length() + include_brackets * 2, ' ')};
  }

  //   1
  // 3 -
  //   2

  numerator = remove_extra_front_back_brackets(numerator);
  denominator = remove_extra_front_back_brackets(denominator);

  bool bracket_numerator =
      include_brackets || (numerator.length() > 1 && numerator[0] == '(' &&
                           get_matching_brace(numerator, 0) == -1);

  size_t last_closing = denominator.rfind(')');
  size_t closing_bracket =
      (last_closing == std::string::npos
           ? last_closing
           : eval_with_steps::get_back_corresponding_bracket(
                 denominator.c_str(), last_closing));

  bool bracket_denominator =
      include_brackets ||
      (last_closing != std::string::npos && denominator.length() > 1 &&
       (last_closing == denominator.length() - 1 ||
        (last_closing + 1 < denominator.length() &&
         denominator[last_closing + 1] == '^')) &&
       closing_bracket == -1);

  numerator = numerator.substr(bracket_numerator && !include_brackets,
                               numerator.length());
  std::string og_denominator = denominator;
  denominator = denominator.substr(
      0, denominator.length() -
             (include_brackets
                  ? 0
                  : (bracket_denominator ? denominator.length() - last_closing
                                         : 0)));

  std::string spaces = std::string(whole.length() + 1, ' ');
  if (whole.empty())
    spaces.pop_back();
  size_t m = std::max(numerator.length(), denominator.length());
  std::vector<std::string> answer = {spaces + center(numerator, m)};
  if (!whole.empty())
    answer.push_back(whole + " ");
  else
    answer.push_back("");
  answer.back() += std::string(m, '-');
  answer.push_back(spaces + center(denominator, m));

  // perform padding
  size_t max_length = 0;
  for (auto &i : answer) {
    if (i.length() > max_length)
      max_length = i.length();
  }
  for (auto &i : answer) {
    if (i.length() < max_length)
      i += std::string((max_length - i.length()), ' ');
  }

  if (bracket_numerator) {
    answer[0] = " " + answer[0];
    answer[1] = "(" + answer[1];
    answer[2] = " " + answer[2];
  }
  if (bracket_denominator) {
    if (include_brackets) {
      answer[0] += " ";
      answer[1] += ")";
      answer[2] += " ";
    } else {
      answer[0] += std::string(denominator.length() - last_closing + 1, ' ');
      answer[1] += og_denominator.substr(last_closing, og_denominator.length());
      answer[2] += std::string(denominator.length() - last_closing + 1, ' ');
    }
  }

  return answer;
}