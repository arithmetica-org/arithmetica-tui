#include <algorithm>
#include <arithmetica.h>
#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <helpers.hpp>

namespace arithmetica_factor_polynomial {
typedef arithmetica::Fraction am_frac;
typedef basic_math_operations::BMONum bmo_num;

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
                                      std::string variable) {
  if (coeffs.empty()) {
    return "0";
  }
  if (coeffs.size() == 1) {
    return arithmetica::to_string(coeffs[0]);
  }
  std::string answer;
  bool reached_nonzero = false;
  for (size_t i = 0; i < coeffs.size(); ++i) {
    am_frac &n = coeffs[i];
    if (n == "0") {
      continue;
    }
    if (reached_nonzero && n.numerator[0] != '-') {
      answer += "+";
    }
    reached_nonzero = true;
    if (n == "-1" && coeffs.size() - i - 1 != 0) {
      answer += "-";
    } else if (!(n == "1")) {
      answer += arithmetica::to_string(n);
    } else if (coeffs.size() - i - 1 == 0) {
      answer += "1";
    }
    if (coeffs.size() - i - 1 != 0) {
      answer += variable;
      if (coeffs.size() - i - 1 != 1) {
        answer += "^" + std::to_string(coeffs.size() - i - 1);
      }
    }
  }
  return answer;
}

std::vector<std::string> multiply_polynomials_with_steps(
    std::vector<am_frac> p1, std::vector<am_frac> p2,
    std::vector<am_frac> &answer, std::string variable) {
  std::string p1_str = form_printable_polynomial(p1, variable);
  std::string p2_str = form_printable_polynomial(p2, variable);

  if (p1_str == "1") {
    answer = p2;
    return {};
  }
  if (p2_str == "1") {
    answer = p1;
    return {};
  }

  // Distribute p1 over p2
  std::vector<std::string> steps;
  std::string step;
  bool reached_nonzero = false;
  for (size_t i = 0; i < p1.size(); ++i) {
    am_frac &n = p1[i];
    if (n == "0") {
      continue;
    }
    if (reached_nonzero && n.numerator[0] != '-') {
      step += "+";
    }
    reached_nonzero = true;
    if (n == "-1" && p1.size() - i - 1 != 0) {
      step += "-";
    } else if (!(n == "1")) {
      step += arithmetica::to_string(n);
    } else if (p1.size() - i - 1 == 0) {
      step += "1";
    }
    if (p1.size() - i - 1 != 0) {
      step += variable;
      if (p1.size() - i - 1 != 1) {
        step += "^" + std::to_string(p1.size() - i - 1);
      }
    }

    step += "(" + p2_str + ")";
  }
  steps.push_back(step);
  step.clear();

  // Max power: p1.size() + p2.size() - 2
  const size_t max_terms =
      p1.size() + p2.size() - 1; // including the constant term
  answer = {};
  for (size_t i = 0; i < max_terms; ++i) {
    answer.push_back("0");
  }
  for (size_t i = 0; i < p1.size(); ++i) {
    am_frac &n = p1[i];
    // current power: p1.size() - i - 1
    // current max power: (p1.size() - i - 1) + (p2.size() - 1)
    //                  = p1.size() + p2.size() - 2 - i
    std::vector<am_frac> partial_result;
    for (auto j = 0; j < max_terms; ++j) {
      partial_result.emplace_back("0");
    }
    for (size_t j = 0; j < p2.size(); ++j) {
      // current power = (p1.size() - i - 1) + (p2.size() - j - 1)
      //               = p1.size() + p2.size() - 2 - i - j
      //               = max_terms - i - j - 1
      // therefore current index = max_terms - 1 - (max_terms - i - j - 1)
      //                         = max_terms - 1 - max_terms + i + j + 1
      //                         = i + j
      partial_result[i + j] = n * p2[j];
    }

    std::string partial_str =
        form_printable_polynomial(partial_result, variable);

    if (i != 0) {
      if (!partial_str.empty() && partial_str[0] != '-') {
        step.push_back('+');
      }
    }
    step += partial_str;

    for (auto j = 0; j < max_terms; ++j) {
      answer[j] = answer[j] + partial_result[j];
    }
  }
  steps.push_back(step);
  steps.push_back(form_printable_polynomial(answer, variable));
  return steps;
}

std::string factor_polynomial(std::string expr, std::vector<std::string> &steps,
                              bool show_steps) {
  arithmetica::algexpr e(expr);
  if (e.is_numeric()) {
    std::cerr << "Error: " << expr << " is not a valid algebraic expression.\n";
    return "ERROR";
  }
  e = e.simplify();

  std::string variable = "?";
  for (auto &i : e.terms()) {
    bool found = false;
    i = i.simplify_term();
    if (i.is_numeric()) {
      continue;
    }
    for (auto &i : i.r->products()) {
      if (i.func == "^") {
        variable = i.l->to_string();
      } else {
        variable = i.to_string();
      }
      found = true;
      break;
    }
    if (found) {
      break;
    }
  }

  if (variable == "?") {
    std::cerr << "Error: " << expr
              << " is not a valid algebraic expression. No variable found.\n";
    return "ERROR";
  }

  for (auto &i : e.terms()) {
    bool failed = false;
    i = i.simplify_term();
    if (i.is_numeric()) {
      continue;
    }
    for (auto &i : i.r->products()) {
      std::string var;
      if (i.func == "^") {
        var = i.l->to_string();
      } else {
        var = i.to_string();
      }
      if (var != variable) {
        failed = true;
        break;
      }
    }
    if (failed) {
      std::cerr << "Error: " << expr
                << " is not a single variable polynomial. Multi variable "
                   "polynomials are not supported yet!\n";
      std::cerr << "Failed on term " << i.to_string() << "\n";
      return "ERROR";
    }
  }

  // Sort the terms by degree.
  auto expr_vec = e.terms();
  auto get_pow = [&](arithmetica::algexpr term, arithmetica::algexpr base) {
    for (auto &i : term.products()) {
      if (i.func == "^") {
        if (*i.l == base) {
          return i.r->coeff;
        }
      } else if (i == base) {
        return arithmetica::Fraction("1");
      }
    }
    return arithmetica::Fraction("0");
  };
  std::sort(expr_vec.begin(), expr_vec.end(),
            [&](const arithmetica::algexpr &a, const arithmetica::algexpr &b) {
              return get_pow(b, arithmetica::algexpr(variable)) <
                     get_pow(a, arithmetica::algexpr(variable));
            });
  for (auto &i : expr_vec) {
    i = i.simplify_term();
  }

  // This will also not work for variable powers
  int _size = std::stoi(
      (bmo_num(
           get_pow(expr_vec[0], arithmetica::algexpr(variable)).to_string()) +
       bmo_num("1"))
          .number);
  std::vector<am_frac> frac_coefficients;
  char **coefficients = (char **)malloc(sizeof(char *) * _size);
  size_t ptr = 0;
  for (int i = 0; i < expr_vec.size(); i++) {
    if (i != 0) {
      std::string s_1 = "0", s_2 = "0";
      if (!expr_vec[i - 1].is_numeric()) {
        s_1 = get_pow(expr_vec[i - 1], arithmetica::algexpr(variable))
                  .to_string();
      }
      if (!expr_vec[i].is_numeric()) {
        s_2 = get_pow(expr_vec[i], arithmetica::algexpr(variable)).to_string();
      }
      auto temp = bmo_num(s_1) - bmo_num(s_2);
      for (int j = 0; j < std::stoi(temp.number) - 1; j++) {
        coefficients[ptr] = (char *)malloc(sizeof(char) * 2);
        strcpy(coefficients[ptr], "0");
        ptr++;
      }
    }

    std::string constant_str = expr_vec[i].l == nullptr
                                   ? expr_vec[i].to_string()
                                   : expr_vec[i].l->to_string();
    coefficients[ptr] =
        (char *)malloc(sizeof(char) * (constant_str.length() + 1));
    strcpy(coefficients[ptr], constant_str.c_str());
    ptr++;
  }

  for (int i = ptr; i < _size; i++) {
    coefficients[ptr] = (char *)malloc(sizeof(char) * 2);
    strcpy(coefficients[ptr], "0");
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
  std::vector<std::vector<am_frac>> factors;
  std::vector<std::string> string_factors;
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

    factors.push_back(coeffs);

    string_factors.push_back("(" + form_printable_polynomial(coeffs, variable) +
                             ")");
    answer += string_factors.back();
  }
  successfully_divided.clear();
  factors.push_back(frac_coefficients);
  string_factors.push_back(
      "(" + form_printable_polynomial(frac_coefficients, variable) + ")");

  if (answer.empty()) {
    answer += form_printable_polynomial(frac_coefficients, variable);
  } else {
    if (form_printable_polynomial(frac_coefficients, variable) != "1") {
      answer +=
          "(" + form_printable_polynomial(frac_coefficients, variable) + ")";
    }
  }

  frac_coefficients.clear();

  if (!show_steps) {
    return answer;
  }

  steps = {};
  std::vector<am_frac> expanded = factors[0];
  for (size_t i = 1; i < factors.size(); ++i) {
    std::vector<am_frac> answer;
    std::vector<std::string> partial_steps =
        multiply_polynomials_with_steps(expanded, factors[i], answer, variable);
    for (auto &j : partial_steps) {
      std::string full_step = "(" + j + ")";
      for (auto k = i + 1; k < string_factors.size(); ++k) {
        if (string_factors[k] != "(1)") {
          full_step += string_factors[k];
        }
      }
      if (full_step.length() > 2) {
        if (get_matching_brace(full_step, 0) == full_step.length() - 1) {
          full_step = full_step.substr(1, full_step.length() - 2);
        }
      }
      steps.push_back(full_step);
    }
    expanded = answer;
  }
  // Remove consecutive duplicates.
  steps.erase(std::unique(steps.begin(), steps.end()), steps.end());
  std::reverse(steps.begin(), steps.end());

  return answer;
}
}; // namespace arithmetica_factor_polynomial