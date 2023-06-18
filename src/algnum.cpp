#include "algnum.hpp"

namespace algnum {
    
algnum algexpr::element(size_t index) { return expr[index]; }
size_t algexpr::size() { return expr.size(); }
void algexpr::insert(algnum n) { expr.push_back(n); }

bool variable::operator==(variable v2) { return var == v2.var && power == v2.power; }
bool variable::operator<(variable v2) { return var < v2.var; }

std::string rfrac_to_latex(rfraction frac) {
  if (frac.denominator != "1")
    return "\\frac{" + frac.numerator + "}{" + frac.denominator + "}";
  else
    return frac.numerator;
};

variable::variable(std::string v, rfraction p) {
  var = v;
  power = p;
};

variable::variable(std::string v, std::string p) {
  var = v;
  power = algexpr(p.c_str());
}

variable::variable(const char *cstr) {
  std::string s = std::string(cstr);
  if (s.find('^') != std::string::npos) {
    var = s.substr(0, s.find('^'));
    std::string rest = s.substr(s.find('^') + 1, s.length());
    if (!rest.empty() && rest[0] == '(')
      power = rest.substr(1, rest.length() - 2);
    else
      power = rest;
  } else {
    var = s;
    power = "1";
  }
};

void algexpr::operator=(const char *cstr) {
  expr.clear();
  expr = algexpr(cstr).expr;
}

void algexpr::operator=(std::string s) {
  expr.clear();
  expr = algexpr(s.c_str()).expr;
}

void algexpr::operator=(rfraction r) {
  expr.clear();
  expr.push_back(algnum(r.to_string().c_str()));
}

bool algnum::operator==(algnum a2) {
  if (!(constant == a2.constant))
    return false;
  if (variables.size() != a2.variables.size())
    return false;
  for (size_t i = 0; i < variables.size(); i++) {
    if (!(variables[i] == a2.variables[i]))
      return false;
  }
  return true;
}

bool algexpr::operator==(algexpr e2) {
  if (size() != e2.size())
    return false;
  for (size_t i = 0; i < size(); i++) {
    if (!(element(i) == e2.element(i)))
      return false;
  }
  return true;
}

static std::vector<std::string> split_string(std::string s, char ch) {
  std::vector<std::string> answer;
  std::string part;
  for (auto &i : s) {
    if (i != ch)
      part.push_back(i);
    else {
      answer.push_back(part);
      part.clear();
    }
  }
  answer.push_back(part);
  return answer;
}

static void replace_all(std::string &str, const std::string &from,
                        const std::string &to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

static bool is_letter(char ch) {
  return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

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

algnum algnum::operator*(algnum a2) {
  algnum answer;
  answer.constant = constant * a2.constant;
  answer.variables = variables;

  for (auto i = 0; i < a2.variables.size(); i++) {
    long long index = -1;
    for (auto j = 0; j < answer.variables.size(); j++) {
      if (answer.variables[j].var == a2.variables[i].var) {
        index = j;
        break;
      }
    }
    if (index == -1) {
      answer.variables.push_back(a2.variables[i]);
    } else {
      answer.variables[index].power =
          answer.variables[index].power + a2.variables[i].power;
    }
  }

  return answer;
}

std::string process_brackets(std::string in, size_t exp_loc, size_t &i);
size_t prev_index(std::string s, size_t start);
size_t next_index(std::string s, size_t start);
bool check_for_functions_behind(std::string input, size_t start_pos,
                                std::vector<std::string> &supportedFunctions);
size_t get_matching_open_brace(std::string str, size_t index);

void remove_all_non_functional_brackets(
    std::string &input, std::vector<std::string> &supportedFunctions) {
  size_t start_pos = 0;
  while ((start_pos = input.find('(', start_pos)) != std::string::npos) {
    if (start_pos == 0) {
      size_t match = get_matching_brace(input, 0);
      if (match == input.length() - 1) {
        input[get_matching_brace(input, 0)] = ' ';
        input[0] = ' ';
        start_pos++;
        continue;
      }
    }
    size_t index_prev_non_space = prev_index(input, start_pos);
    bool can_remove_brackets = input[index_prev_non_space] != '^';
    if (can_remove_brackets) { // only perform this additional
      // check if first one is true
      // check_for_functions() returns true if a function immediately preceeds
      // the bracket start_pos is an index to
      can_remove_brackets =
          !check_for_functions_behind(input, start_pos, supportedFunctions);
    }

    // we can't remove brackets if there's a power operator immediately after,
    // as that might change the meaning of the expression, eg: (a^b)^c != a^b^c
    if (can_remove_brackets) {
      if (input[next_index(input, get_matching_brace(input, start_pos))] ==
          '^') {
        can_remove_brackets = false;
      }
    }

    // finally, if it's still true, perform a check for functions' weird power
    // notations i.e. how tan(x)^2 is written as tan^2(x) or x^(2/3) *can* be
    // written as cbrt^2(x)
    if (can_remove_brackets) {
      // this variable contains the index of the first non-space character
      // or 0 if there are no non space characters
      if (start_pos != 0 && input[index_prev_non_space] == ')') {
        size_t matching_brace_pos =
            get_matching_open_brace(input, index_prev_non_space);
        if (input[prev_index(input, matching_brace_pos)] == '_') {
          can_remove_brackets = false;
        }
        if (input[prev_index(input, matching_brace_pos)] == '^') {
          can_remove_brackets = !check_for_functions_behind(
              input, prev_index(input, matching_brace_pos), supportedFunctions);
        }
      } else {
        // not ')', check gets a little more complicated
        bool prev_enc_let = false, prev_enc_num = false;
        bool encountering_number = false, encountered_number = false,
             encountered_letter = false;
        for (size_t i = index_prev_non_space; i + 1 > 0; i--) {
          // if two adjacent characters are letters and numbers and stricly
          // that (i.e. num w/ num or letter w/ letter won't work), then no
          // function can possibly be inserted with reference to the bracket
          // pair we're considering, so it's safe to replace it with spaces.
          bool is_current_let = is_letter(input[i]),
               is_current_num = '0' <= input[i] && input[i] <= '9';
          if ((prev_enc_let && is_current_num) ||
              (prev_enc_num && is_current_let))
            break;

          if (is_current_num)
            encountering_number = true;
          if (is_current_let) {
            encountered_letter = true;
            if (encountering_number) {
              encountering_number = false;
              encountered_number = true;
            }
          }

          if (!(encountered_letter && encountered_number)) {
            // can_remove_brackets depends on whether the thing before the
            // exponent is a function in the case of an exponent. you can't
            // remove the brackets in the case of an underscore
            if (input[i] == '_') {
              can_remove_brackets = false;
              break;
            }
            if (input[i] == '^') {
              can_remove_brackets =
                  !check_for_functions_behind(input, i, supportedFunctions);
              if (!can_remove_brackets)
                break;
            }
          }

          prev_enc_let = is_current_let;
          prev_enc_num = is_current_num;
        }
      }
    }

    if (can_remove_brackets) {
      input[get_matching_brace(input, start_pos)] =
          ' '; // separate it into parts
      input[start_pos] = ' ';
    }

    start_pos++;
    if (start_pos >= input.length() - 1)
      break;
  }
}

std::vector<std::string> get_supported_functions() {
  std::vector<std::string> supportedFunctions = {"sqrt", "cbrt", "log"};
  for (std::string &i :
       std::vector<std::string>{"sin", "cos", "tan", "csc", "sec", "cot"}) {
    supportedFunctions.push_back(i);
    supportedFunctions.push_back(i + "h");
    supportedFunctions.push_back("arc" + i);
    supportedFunctions.push_back("arc" + i + "h");
  }
  return supportedFunctions;
}

algnum algnum::operator+(algnum a2) {
  // this function only works if both are like
  // it doesn't check this and adds them anyway, be warned.

  algnum answer;
  answer.constant = constant + a2.constant;
  answer.variables = variables;

  return answer;
};

void process_power(std::string &in, size_t &j) {
  for (; j < in.length(); ++j) {
    // if the current character is a '(', we can skip ahead to the
    // matching ')'
    if (in[j] == '(') {
      j = get_matching_brace(in, j) + 1;
      continue;
    }

    // we can break if the current character is a letter and the next
    // is BOTH not a letter and not a '^'
    if (is_letter(in[j]) &&
        (j + 1 < in.length() && !is_letter(in[j + 1]) && in[j + 1] != '^')) {
      break;
    }
    // if the current character is a number and the next is a letter
    // we can break
    if ('0' <= in[j] && in[j] <= '9' && j + 1 < in.length() &&
        is_letter(in[j + 1])) {
      break;
    }

    // check if there's a non '^' symbol ahead, if so we're done
    if (j + 1 < in.length() && in[j + 1] != '^') {
      break;
    } else {
      j++;
    }
  }
}

algnum::algnum(const char *s) {
  std::string input = std::string(s);

  /*
  Functional requirements:

    1. Identify any brackets ('{}', '()', '[]')

    2. Detect multiplication indicated by either (x)(y), (x)y, x(y), x*y, or
    just xy.

    3. Detect some special functions such as sqrt(), cbrt(), sin(), cos(),
    tan(), sec(), cot(), csc() / cosec(), all trig inverse functions such as
    sin^(-1)() which is the same thing as arcsin() which shouldn't be mistaken
    for multiplication, ln(), log_b() should be identified as log to the base
    b, log() (assume base e).

    4. Detect constants such as 7^(2/5) and assign them as a variable (so they
    only get added with other like constants).

    5. Detect rational constants in the middle of variables (such as x^2(2)y,
    where 2 is a constant which should be multiplied w/ the already existing
    constant part).

    6. Detect like terms that have already been made (such as x^2yx, should
    detect the repeated x and not make repeated variables.

    7. sqrt^2(x), cbrt^2(x), and other obscure function notations should be
    supported.

    Note: tanx should be parsed as tan(x), tanx+2 should be parsed as tan(x) +
    2, tanx2 should be parsed as 2*tan(x) but tan2x should be parsed as
    tan(2x).
  */

  // Deal with alternative brackets (i.e. anything other than '()')
  replace_all(input, "{", "(");
  replace_all(input, "}", ")");
  replace_all(input, "[", "(");
  replace_all(input, "]", ")");

  // cosec and csc mean the same thing. Also, ln and log without an explicit
  // base also mean the same thing.
  replace_all(input, "cosec", "csc");
  replace_all(input, "ln", "log");

  // Deal with inverse trig. functions
  for (std::string &i :
       std::vector<std::string>{"sin", "cos", "tan", "csc", "sec", "cot"}) {
    replace_all(input, i + "^(-1)", "arc" + i);
    replace_all(input, i + "h^(-1)", "arc" + i + "h"); // Hyperbolic
  }

  // I'm also choosing to ignore all empty brackets (i.e. '()').
  replace_all(input, "()", "");

  // List of supported functions
  auto supportedFunctions = get_supported_functions();

  // Now, let's eliminate the problem of variables followed by '-' such as
  // '-y'
  for (auto i = 0; i < input.length() - 1; i++) {
    if (input[i] == '-' && is_letter(input[i + 1])) {
      input.insert(i + 1, "1");
    }
  }

  // To deal with the second condition, we can eliminate all brackets that
  // don't enclose the parameters of a function
  remove_all_non_functional_brackets(input, supportedFunctions);

  // Remove all '*' symbols that separate variables, for example "x*y" -> "xy"
  for (auto i = 0; i < input.length(); i++) {
    if (input[i] == '*' && i != 0 && i < input.length() - 1 &&
        is_letter(input[i - 1]) && is_letter(input[i + 1])) {
      input.erase(i, 1);
    }
  }

  replace_all(input, "*", " "); // for 2.

  // Now, remove all subsequent spaces, for example "  hell  o " -> " hell o "
  while (input.find("  ") != std::string::npos)
    replace_all(input, "  ", " ");

  // Deal with each part of the input individually.
  constant = "1"; // since no constant (i.e. x^2y) implies the constant is 1
  auto temp = split_string(input, ' ');
  for (auto &in : temp) {
    for (size_t i = 0; i < in.length(); i++) {
      // check for functions first
      bool functionExists = false;
      std::string functionName;
      for (auto &func : supportedFunctions) {
        if (in.substr(i, func.length()) == func) {
          functionExists = true;
          functionName = func;
          break;
        }
      }

      if (functionExists) {
        std::string varparam;
        std::string varpower = "1"; // default power

        // deal with functions
        // first, let's deal with the obscure function power notation
        if (in[i + functionName.length()] == '^') {
          size_t powerLocation = i + functionName.length();
          // case with braces is really easy
          if (in[powerLocation + 1] == '(') {
            size_t matching_brace = get_matching_brace(in, powerLocation + 1);
            varpower = in.substr(powerLocation + 2,
                                 matching_brace - powerLocation - 2);
            size_t next_matching_brace =
                get_matching_brace(in, matching_brace + 1);
            varparam = in.substr(matching_brace + 2,
                                 get_matching_brace(in, matching_brace + 1) -
                                     matching_brace - 2);
            i = next_matching_brace;
          } else {
            // this case is also not too bad, we just look ahead for the first
            // '(', that's where our function's parameter is, and will be
            // where the power part ends
            size_t param_brac_begin = in.find('(', i);
            size_t matching_brace = get_matching_brace(in, param_brac_begin);
            varpower = in.substr(powerLocation + 1,
                                 param_brac_begin - powerLocation - 1);
            varparam = in.substr(param_brac_begin + 1,
                                 matching_brace - param_brac_begin - 1);
            i = matching_brace;
          }
        } else {
          // we don't need to set the power, default is already 1
          size_t bracket_location = in.find('(', i);
          size_t matching_brace = get_matching_brace(in, bracket_location);
          varparam = in.substr(bracket_location + 1,
                               matching_brace - bracket_location - 1);
          i = matching_brace;
        }

        // special functions
        // if varparam doesn't contain any letters, it's a constant
        bool contains_letters = false;
        for (auto &i : varparam) {
          if (is_letter(i)) {
            contains_letters = true;
            break;
          }
        }
        if (functionName == "sqrt") {
          varpower = varpower + "*1/2";
          variable v = variable(varparam, varpower);
          v.constant = !contains_letters;
          add_variable(v);
        } else if (functionName == "cbrt") {
          varpower = varpower + "*1/3";
          variable v = variable(varparam, varpower);
          v.constant = !contains_letters;
          add_variable(v);
        } else {
          variable v = variable(functionName + "(" + varparam + ")", varpower);
          v.function = true;
          v.functionName = functionName;
          v.functionValue = varparam;
          add_variable(v);
        }
      }

      else if (is_letter(in[i])) {
        std::string varname;
        std::string varpower = "1";

        // we've come across a variable
        size_t variable_begin = i;
        // a variable can either have only a single letter, or a letter
        // followed by a subscript '_' with the subscript either having or not
        // having brackets around it

        // first, let's deal with the subscript variables
        // currently, this code breaks if spaces are used with the
        // subscript variables
        if (variable_begin + 1 < in.length() && in[variable_begin + 1] == '_') {
          size_t subscript_location = variable_begin + 1;

          // if the subscript is enclosed by brackets
          if (subscript_location + 1 < in.length() &&
              in[subscript_location + 1] == '(') {
            size_t first_brace_location = subscript_location + 1;
            size_t matching_brace =
                get_matching_brace(in, first_brace_location);
            varname = in.substr(i, matching_brace - i + 1);
            i = matching_brace + 1;
          } else {
            // the subscript is not enclosed in brackets
            // if the first character of the subscript is a letter, we know
            // that that's where the variable name ends
            if (subscript_location + 1 < in.length() &&
                is_letter(subscript_location + 1)) {
              varname = in.substr(i, subscript_location - i + 2);
              i = subscript_location + 2;
            } else {
              // keep going ahead till a non-numerical character is
              // encountered
              varname = in.substr(i, 2);
              for (auto j = subscript_location + 1; j < in.length(); j++) {
                if (!('0' <= in[j] && in[j] <= '9')) {
                  i = j;
                  break;
                }
                varname.push_back(in[j]);
              }
            }
          }
        } else {
          // this means we don't have a subscript variable, the variable here
          // is just this singular letter
          varname = in.substr(i, 1);
          i++;
        }

        // now, let's deal with the variables' powers
        // a variable only has a power if the current character (since we
        // incremented i while setting the variable's name) is '^'
        if (i < in.length() && in[i] == '^') {
          size_t powerLocation = i;
          // again, only a letter means we're done and that letter is the
          // power - hi its me from 6 months later, no it doesn't, for eg
          // a^b^c ==> pow(a, b^c)
          size_t j = powerLocation + 1;
          process_power(in, j);
          // powerLocation+1 to j -> varpower
          varpower = in.substr(powerLocation + 1, j - powerLocation);
          i = j;
        } else {
          if (varname.length() == 1)
            i--; // since we incremented previously for a single letter
                 // variable
        }

        add_variable(variable(varname, varpower));
      }
      /*
      221^(1/3), this is only a variable if it's raised to a power. keep
      checking forward, if you encounter a letter then it's a constant. if you
      encounter ^ before a letter, it's a variable.
      */
      else if (('0' <= in[i] && in[i] <= '9') || in[i] == '-') {
        std::string num;
        // this could either be a constant or a power raised variable
        // (221^(1/3))
        bool did_break = false;
        for (auto j = i; j < in.length(); j++) {
          if (is_letter(in[j])) {
            // is a constant
            i = j - 1;
            constant = constant * rfraction(num.c_str());
            did_break = true;
            break;
          }
          if (in[j] == '^') {
            // is a variable
            size_t t = j;
            ++j;
            process_power(in, j);
            std::string power = in.substr(t + 1, j - t - 1);
            variable v = variable(num, power);
            v.constant = true;
            add_variable(v);
            did_break = true;
            i = j;
            break;
          }
          num.push_back(in[j]);
        }
        if (!did_break) {
          constant = constant * rfraction(num.c_str());
          i += num.length() - 1;
        }
      } else if (in[i] == '(') {
        /// deal with the '(a+b)^(c+d) case' case
        size_t matching_brace = get_matching_brace(in, i);
        size_t j = matching_brace + 2;
        process_power(in, j);
        std::string power =
            in.substr(matching_brace + 2, j - matching_brace - 1);
        std::string name = in.substr(i + 1, matching_brace - i - 1);
        variable v = variable(name, power);
        v.constant = false;
        add_variable(v);
        i = j;
      }
    }
  }

  // Finally, simplify the constants which have mixed fraction powers
  std::vector<variable> newVariables;
  for (auto i = 0; i < variables.size(); i++) {
    variable &var = variables[i];
    if (var.constant) {
      if (false) {
        // if ((var.power.numerator - var.power.denominator).to_string()[0] !=
        // '-') {
        //   // if the numerator is greater than or equal to the denominator
        //   size_t temp1 = var.power.numerator.division_accuracy,
        //          temp2 = var.power.denominator.division_accuracy;
        //   var.power.numerator.division_accuracy = 0;
        //   var.power.denominator.division_accuracy = 0;
        //   rnumber integerPart = var.power.numerator /
        //   var.power.denominator; var.power.numerator.division_accuracy =
        //   temp1; var.power.denominator.division_accuracy = temp2; rfraction
        //   multiplier = "1"; rfraction toMultiplyWith =
        //   rfraction(var.var.c_str()); var.power = var.power -
        //   rfraction(integerPart, rnumber("1")); if (var.power.numerator !=
        //   "0")
        //     newVariables.push_back(var);
        //   while (integerPart.number != "0") {
        //     multiplier = multiplier * toMultiplyWith;
        //     integerPart = integerPart - "1";
        //   }
        //   constant = constant * multiplier;
        // } else
      } else
        newVariables.push_back(var);
    } else
      newVariables.push_back(var);
  }

  // Remove the raised to 0 variables.
  variables = newVariables;
}

std::string algnum::latex() {
  if (constant.numerator == "0")
    return "0";
  std::string answer;
  if (!variables.empty()) {
    if (constant.numerator != constant.denominator) {
      if (constant.numerator == "-1")
        answer += "-";
      else
        answer += rfrac_to_latex(constant);
    }
  } else {
    answer += rfrac_to_latex(constant);
  }
  for (auto &var : variables) {
    if (var.function) {
      answer += "\\" + var.functionName;
      if (!var.power.latex().empty() && var.power.latex() != "1") {
        // if var.power.latex() is only numbers, then we don't need brackets
        if (var.power.latex().find_first_not_of("0123456789") ==
            std::string::npos) {
          answer += "^" + var.power.latex();
        } else {
          answer += "^{" + var.power.latex() + "}";
        }
      }
      answer += "{" + var.functionValue.latex() + "}";
    } else {
      if (var.var.length() > 1)
        answer += "{" + var.var + "}";
      else
        answer += var.var;
      if (!var.power.latex().empty() && var.power.latex() != "1") {
        if (var.power.latex().find_first_not_of("0123456789") ==
            std::string::npos) {
          answer += "^" + var.power.latex();
        } else {
          answer += "^{" + var.power.latex() + "}";
        }
      }
    }

    answer += " ";
  }

  if (!variables.empty())
    answer.pop_back();

  return answer;
};

void algnum::add_variable(variable v) {
  for (auto &i : variables) {
    if (i.var == v.var) {
      i.power = i.power + v.power;
      return;
    }
  }
  variables.push_back(v);
}

std::string process_brackets(std::string in, size_t exp_loc, size_t &i) {
  if (exp_loc + 1 >= in.length())
    return "";
  // if we have brackets
  if (in[exp_loc + 1] == '(') {
    size_t first_brace_location = exp_loc + 1;
    size_t matching_brace = get_matching_brace(in, first_brace_location);
    i = matching_brace;
    return in.substr(first_brace_location + 1,
                     matching_brace - first_brace_location - 1);
  } else {
    if (is_letter(in[exp_loc + 1]))
      return in.substr(exp_loc + 1, 1);
    std::string answer;
    for (auto j = exp_loc + 1; j < in.length(); j++) {
      if (!('0' <= in[j] && in[j] <= '9')) {
        i = j - 1;
        break;
      }
      answer.push_back(in[j]);
    }
    return answer;
  }
};

size_t prev_index(std::string s, size_t start) {
  if (start == 0)
    return 0;
  for (auto i = start - 1; i + 1 > 0; i--)
    if (s[i] != ' ')
      return i;
  return 0;
};

size_t next_index(std::string s, size_t start) {
  if (start == s.length() - 1)
    return s.length() - 1;
  for (auto i = start + 1; i < s.length(); i++)
    if (s[i] != ' ')
      return i;
  return s.length() - 1;
};

bool check_for_functions_behind(std::string input, size_t start_pos,
                                std::vector<std::string> &supportedFunctions) {
  for (auto &func : supportedFunctions) {
    if ((long long)start_pos - (long long)func.length() >= 0) {
      if (input.substr(start_pos - func.length(), func.length()) == func) {
        return true;
      }
    }
  }
  return false;
};

size_t get_matching_open_brace(std::string str, size_t index) {
  if (str[index] != ')')
    return -1;
  int count = 0;
  for (size_t i = index; i + 1 > 0; i--) {
    if (str[i] == '(')
      count--;
    if (str[i] == ')')
      count++;

    if (!count)
      return i;
  }
  return -1;
}

std::ostream &operator<<(std::ostream &os, const algnum n) {
  if (n.constant.numerator == "0") {
    os << std::string("0");
    return os;
  }
  if (n.variables.empty()) {
    rfraction m = n.constant;
    os << m.to_string();
  }
  if (n.constant.numerator != n.constant.denominator) {
    if (!n.variables.empty())
      os << std::string(" ");
  }
  for (auto i = 0; i < n.variables.size(); i++) {
    os << n.variables[i].var;
    os << std::string("^");
    os << std::string("(") << n.variables[i].power << std::string(")");
    if (i != n.variables.size() - 1)
      os << std::string(" ");
  }
  return os;
}

bool is_like(algnum a, algnum b) {
  std::sort(a.variables.begin(), a.variables.end());
  std::sort(b.variables.begin(), b.variables.end());

  if (a.variables.size() != b.variables.size())
    return false;

  for (auto i = 0; i < a.variables.size(); i++) {
    if (a.variables[i].var != b.variables[i].var)
      return false;
    if (!(a.variables[i].power == b.variables[i].power))
      return false;
  }

  return true;
}

algexpr algexpr::operator*(algexpr e2) {
  algexpr answer;
  for (auto i = 0; i < e2.size(); i++) {
    for (auto j = 0; j < expr.size(); j++) {
      answer.insert(e2.element(i) * expr[j]);
    }
  }

  return combine_like_terms(answer);
}

algexpr algexpr::operator+(algexpr e2) {
  algexpr answer;
  answer.expr = expr;

  for (auto i = 0; i < e2.size(); i++) {
    answer.insert(e2.element(i));
  }
  return combine_like_terms(answer);
}

algexpr algexpr::combine_like_terms(algexpr e) {
  algexpr answer;
  std::vector<size_t> added;
  for (auto i = 0; i < e.size(); i++) {
    if (std::find(added.begin(), added.end(), i) == added.end()) {
      algnum temp = e.element(i);
      for (auto j = i + 1; j < e.size(); j++) {
        if (is_like(temp, e.element(j))) {
          if (std::find(added.begin(), added.end(), j) == added.end()) {
            temp = temp + e.element(j);
            added.push_back(j);
          }
        }
      }
      if (!(temp.constant == "0")) {
        answer.insert(temp);
      }
    }
  }
  return answer;
}

std::string algexpr::latex() {
  std::string answer;
  for (auto i = 0; i < expr.size(); i++) {
    answer += expr[i].latex();
    if (i + 1 < expr.size()) {
      std::string temp = expr[i + 1].latex();
      if (!temp.empty() && temp[0] != '-')
        answer += "+";
    }
  }
  return answer;
}

static std::vector<std::string> split_string_for_algexpr(std::string s,
                                                         char ch) {
  std::vector<std::string> answer;
  std::string part;
  int count = 0;
  for (auto &i : s) {
    if (i == '(')
      count++;
    if (i == ')')
      count--;
    if (count != 0 || i != ch)
      part.push_back(i);
    if (count == 0 && i == ch) {
      answer.push_back(part);
      part.clear();
    }
  }
  answer.push_back(part);
  return answer;
}

algexpr::algexpr(const char *s) {
  std::string input = std::string(s);
  clean_double_signs(input);
  auto supportedFunctions = get_supported_functions();
  remove_all_non_functional_brackets(input, supportedFunctions);

  
  while (input.find(" - ") != std::string::npos) {
    replace_all(input, " - ", "-");
  }

  replace_all(input, "-", "+-");

  std::vector<std::string> numbers = split_string_for_algexpr(input, '+');
  for (auto &i : numbers) {
    if (i.empty())
      continue;

    clean_double_signs(i);
    expr.emplace_back(i.c_str());
  }
};

void algexpr::clean_double_signs(std::string &expression) {
  while ((expression.find("--") != std::string::npos) ||
         (expression.find("++") != std::string::npos) ||
         (expression.find("-+") != std::string::npos) ||
         (expression.find("+-") != std::string::npos)) {
    replace_all(expression, "--", "+");
    replace_all(expression, "-+", "-");
    replace_all(expression, "+-", "-");
    replace_all(expression, "++", "+");
  }
  replace_all(expression, "*+", "*");
  replace_all(expression, "/+", "/");
};

std::ostream &operator<<(std::ostream &os, const algexpr n) {
  std::string temp;
  if (n.expr.empty())
    return os;
  for (auto i = 0; i < n.expr.size(); i++) {
    if (n.expr[i].constant.numerator[0] != '-')
      os << n.expr[i];
    else {
      algnum n1;
      n1.constant = n.expr[i].constant.numerator.substr(
          1, n.expr[i].constant.numerator.length());
      n1.variables = n.expr[i].variables;
      os << n1;
    }
    if (i + 1 < n.expr.size()) {
      if (n.expr[i + 1].constant.numerator[0] != '-')
        os << std::string(" + ");
      else
        os << std::string(" - ");
    }
  }
  return os;
}
}