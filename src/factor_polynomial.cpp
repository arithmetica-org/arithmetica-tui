#include "algebraic_number_class.hpp"
#include <arithmetica.h>
#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace arithmetica_factor_polynomial {
typedef arithmetica::Fraction am_frac;
typedef basic_math_operations::BMONum bmo_num;

// Create a power compare function for algebraic_term to help sort the terms.
bool power_compare(const algebraic_num::algebraic_number &a,
                   const algebraic_num::algebraic_number &b) {
  // Note that we're assuming each term only has one (or none) variable, and
  // that the variable is the same.

  // We are also assuming that the degrees are *never* the same.

  return a.variablePart.begin()->second > b.variablePart.begin()->second;
}

std::vector<am_frac> divide_polynomial(const std::vector<am_frac> &coefficients,
                                       am_frac root, am_frac &remainder) {
  std::vector<am_frac> answer;
  remainder = "0";
  for (size_t i = 0; i < coefficients.size(); ++i) {
    am_frac n = coefficients[i];
    remainder = n + remainder * root;
    if (i != coefficients.size() - 1) {
      answer.push_back(remainder);
    }
  }
  return answer;
}

std::string form_printable_polynomial(std::vector<am_frac> &coeffs,
                                      char variable) {
  if (coeffs.empty()) {
    return "0";
  }
  if (coeffs.size() == 1) {
    return arithmetica::to_string(coeffs[0]);
  }
  std::string answer;
  for (size_t i = 0; i < coeffs.size(); ++i) {
    am_frac &n = coeffs[i];
    if (n == "0") {
      continue;
    }
    if (i != 0 && n.numerator[0] != '-') {
      answer += "+";
    }
    if (!(n == "1")) {
      answer += arithmetica::to_string(n);
    } else if (coeffs.size() - i - 1 == 0) {
      answer += "1";
    }
    if (coeffs.size() - i - 1 != 0) {
      answer += std::string(1, variable);
      if (coeffs.size() - i - 1 != 1) {
        answer += "^" + std::to_string(coeffs.size() - i - 1);
      }
    }
  }
  return answer;
}

std::string factor_polynomial(std::string expr) {
  auto terms = algebraic_num::get_terms(expr);
  if (terms.empty()) {
    std::cerr << "Error: " << expr << " is not a valid algebraic expression.\n";
    return "ERROR";
  }

  // Let's remove all terms that have their constant part as '0' and their
  // variable part empty.
  for (int i = 0; i < terms.size(); i++) {
    if (terms[i].constantPart == "0" && terms[i].variablePart.empty()) {
      terms.erase(terms.begin() + i);
      i--;
    }
  }

  char variable = '?';
  for (auto &i : terms) {
    bool found = false;
    for (auto &i : i.variablePart) {
      variable = i.first;
      found = true;
      break;
    }
    if (found) {
      break;
    }
  }

  if (variable == '?') {
    std::cerr << "Error: " << expr
              << " is not a valid algebraic expression. No variable found.\n";
    return "ERROR";
  }

  for (auto &i : terms) {
    bool failed = false;
    for (auto &i : i.variablePart) {
      if (i.first != variable) {
        failed = true;
        break;
      }
    }
    if (i.variablePart.size() > 1 || failed) {
      std::cerr << "Error: " << expr
                << " is not a single variable polynomial. Multi variable "
                   "polynomials are not supported yet!\n";
      std::cerr << "Failed on term " << i.get_formatted_number() << "\n";
      return "ERROR";
    }
  }

  // Sort the terms by degree.
  std::sort(terms.begin(), terms.end(), power_compare);

  int _size = terms[0].variablePart.begin()->second + 1;
  std::vector<am_frac> frac_coefficients;
  char **coefficients = (char **)malloc(sizeof(char *) * _size);
  float power = terms[0].variablePart.begin()->second + 1;
  size_t ptr = 0;
  for (int i = 0; i < terms.size(); i++) {
    if (i != 0) {
      for (int j = 0; j < terms[i - 1].variablePart.begin()->second -
                              terms[i].variablePart.begin()->second - 1;
           j++) {
        coefficients[ptr] = (char *)malloc(sizeof(char) * 2);
        strcpy(coefficients[ptr], "0");
        ptr++;
      }
    }

    coefficients[ptr] =
        (char *)malloc(sizeof(char) * terms[i].constantPart.length());
    strcpy(coefficients[ptr], terms[i].constantPart.c_str());
    ptr++;
  }

  size_t answer_size;
  struct fraction **roots = find_roots_of_polynomial(
      (const char **)coefficients, _size, &answer_size);

  std::vector<am_frac> roots_str;
  for (int i = 0; i < answer_size; i++) {
    roots_str.push_back(am_frac(roots[i]->numerator, roots[i]->denominator));
    delete_fraction(*roots[i]);
    free(roots[i]);
  }

  free(roots);
  for (int i = 0; i < _size; i++) {
    frac_coefficients.push_back(am_frac(coefficients[i]));
    free(coefficients[i]);
  }
  free(coefficients);

  std::vector<am_frac> successfully_divided;
  for (int i = 0; i < roots_str.size(); i++) {
    am_frac remainder = "0";

    while (remainder == "0") {
      am_frac root = roots_str[i];
      auto divided = divide_polynomial(frac_coefficients, root, remainder);
      if (remainder == "0") {
        successfully_divided.push_back(root);
        frac_coefficients = divided;
      }
    }
  }

  std::string answer;
  for (auto &i : successfully_divided) {
    std::vector<am_frac> coeffs = {"1", i * "-1"};

    bool cond_1 = coeffs[1].denominator != "1";
    bool cond_2 = coeffs[0].denominator != "1";
    if (cond_1 || cond_2) {
      std::string factor =
          cond_1 ? coeffs[1].denominator : coeffs[0].denominator;
      coeffs[0] = coeffs[0] * factor;
      coeffs[1] = coeffs[1] * factor;

      // Also divide the frac_coefficients by the denominator to even out the
      // multiplication.
      for (auto &i : frac_coefficients) {
        i = i / factor;
      }
    }

    answer += "(" + form_printable_polynomial(coeffs, variable) + ")";
  }
  if (answer.empty()) {
    answer += form_printable_polynomial(frac_coefficients, variable);
  } else {
    if (form_printable_polynomial(frac_coefficients, variable) != "1") {
      answer +=
          "(" + form_printable_polynomial(frac_coefficients, variable) + ")";
    }
  }

  return answer;
}
}; // namespace arithmetica_factor_polynomial