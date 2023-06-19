#include "helpers.hpp"

void print_eval_expression(std::string expression, int outputType, int padding,
                           std::vector<std::string> *outTerms,
                           std::vector<std::string> *outSigns) {
  expression = remove_extra_front_back_brackets(expression);

  size_t index = expression.rfind("==> ");
  std::string beginning_to_print;
  if (index != std::string::npos) {
    beginning_to_print = expression.substr(0, index + 4);
    expression = expression.substr(index + 4, expression.length());
    padding += index + 4;
  }

  std::vector<std::string> terms;
  std::vector<std::string> signs;
  std::string left, right;

  std::vector<std::string> rightSigns, rightTerms;

  int bracket_count = 0;

  bool negative_sign_at_index_0 =
      expression.length() > 0 && expression[0] == '-';

  for (size_t i = 0; i < expression.length(); ++i) {
    if (expression[i] == ' ')
      continue;

    if (expression[i] == '(') {
      bracket_count++;
    }
    if (expression[i] == ')') {
      bracket_count--;
    }

    if (bracket_count != 0) {
      continue;
    }

    if (expression[i] == '*' || expression[i] == '+' || expression[i] == '-' ||
        expression[i] == '^') {
      if (expression[i] == '-') {
        long operational_sign = eval_with_steps::find_operational_sign(
            expression.c_str(), expression[i]);
        if (operational_sign != i) {
          if (i != 0) {
            continue;
          }
        }
      }
      if (expression[i] == '+') {
        if (i == 0) {
          continue;
        }
      }

      char temp = expression[i];
      if (expression[i] == '*') {
        expression[i] = '+'; // I'm doing this instead of fixing the bug in
                             // get_numerical_arguments because yes
      }

      long start = i, end = i;
      char *leftArgument = eval_with_steps::get_numerical_arguments(
          expression.c_str(), true, &start, outputType);
      char *rightArgument = eval_with_steps::get_numerical_arguments(
          expression.c_str(), false, &end, outputType);

      expression[i] = temp;

      left = leftArgument, right = rightArgument;
      free(leftArgument);
      free(rightArgument);

      if (((!signs.empty() && !negative_sign_at_index_0) ||
           (negative_sign_at_index_0 && signs.size() > 1)) &&
          signs.back() == "-") {
        left = left.substr(1, left.length());
      }

      if (left.find_first_of("+-*") != std::string::npos) {
        std::vector<std::string> left_signs;
        std::vector<std::string> left_terms;
        print_eval_expression(left, outputType, padding, &left_terms,
                              &left_signs);

        bool signs_in_left = left.find_first_of("+-*") != std::string::npos;

        bool frac_cond = false;
        size_t index =
            (left_terms.empty() ? std::string::npos
                                : left_terms[0].find_first_of("+-*"));
        if (!left_terms.empty() && (index == std::string::npos ||
                                    (index == 0 && left_terms[0][0] == '-'))) {
          arithmetica::Fraction f;
          std::string s = left_terms[0];
          replace_all(s, "(", "");
          replace_all(s, ")", "");
          if (!left_terms.empty()) {
            f = s;
          }
          frac_cond = (f.numerator[0] == '-' || f.denominator != "1");
        }

        if ((left.length() >= 2 && left[0] == '(' &&
             get_matching_brace(left, 0) == left.length() - 1) ||
            (expression[i] == '^' &&
             (left_terms.size() > 1 || (!left_terms.empty() && frac_cond))) ||
            (expression[i] == '^' && signs_in_left)) {
          if (left_terms.size() > 1) {
            left_terms[0] = "(" + left_terms[0];
            left_terms.back() += ")";
          } else {
            left_terms[0] = "(" + left_terms[0] + ")";
          }
        }

        terms.insert(terms.end(), left_terms.begin(), left_terms.end());
        signs.insert(signs.end(), left_signs.begin(), left_signs.end());
      } else {
        terms.push_back(left);
      }

      if (right.find_first_of("+-*") != std::string::npos) {
        print_eval_expression(right, outputType, padding, &rightTerms,
                              &rightSigns);

        bool signs_in_right = right.find_first_of("+-*") != std::string::npos;

        bool frac_cond = false;
        size_t index =
            (rightTerms.empty() ? std::string::npos
                                : rightTerms[0].find_first_of("+-*"));
        if (!rightTerms.empty() && (index == std::string::npos ||
                                    (index == 0 && rightTerms[0][0] == '-'))) {
          arithmetica::Fraction f;
          std::string s = rightTerms[0];
          replace_all(s, "(", "");
          replace_all(s, ")", "");
          if (!rightTerms.empty()) {
            f = s;
          }
          frac_cond = (f.numerator[0] == '-' || f.denominator != "1");
        }

        if ((right.length() >= 2 && right[0] == '(' &&
             get_matching_brace(right, 0) == right.length() - 1) ||
            (expression[i] == '^' &&
             (rightTerms.size() > 1 || (!rightTerms.empty() && frac_cond))) ||
            (expression[i] == '^' && signs_in_right)) {
          if (rightTerms.size() > 1) {
            rightTerms[0] = "(" + rightTerms[0];
            rightTerms.back() += ")";
          } else {
            rightTerms[0] = "(" + rightTerms[0] + ")";
          }
        }
      }

      std::string sign = std::string(1, expression[i]);
      if (sign == "*") {
        sign = "\u00d7";
      }
      signs.push_back(sign);
      i = end;
    }
  }

  if (terms.empty()) {
    terms.push_back(expression);
  } else {
    if (right.find_first_of("+-*") != std::string::npos) {
      terms.insert(terms.end(), rightTerms.begin(), rightTerms.end());
      signs.insert(signs.end(), rightSigns.begin(), rightSigns.end());
    } else {
      terms.push_back(right);
    }
  }

  if (outTerms != NULL) {
    *outTerms = terms;
  }
  if (outSigns != NULL) {
    *outSigns = signs;
  }

  if (outTerms != NULL && outSigns != NULL) {
    return;
  }

  std::vector<std::string> out;
  print_expression(terms, signs, padding, NULL, &out, 1);

  for (auto &i : out) {
    // Repalce all '*''s with '\u00d7' (multiplication sign)
    replace_all(i, "*", "\u00d7");
  }

  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i].empty() || out[i].find_first_not_of(' ') == std::string::npos) {
      continue;
    }
    if (i == 1) {
      std::cout << beginning_to_print;
    }
    std::cout << out[i] << "\n";
  }
}
