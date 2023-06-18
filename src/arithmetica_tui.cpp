#include "algnum.hpp"
#include <algorithm>
#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#ifdef __linux__
#include <sys/ioctl.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
int get_console_width() {
#ifdef __linux__
  int ans = 20;
  struct winsize w;
  memset(&w, 0, sizeof(w));
  w.ws_col = 0;
  w.ws_row = 0;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  if (w.ws_col > 0) {
    ans = w.ws_col;
  }
  return ans;
#endif
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#endif
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

// Get a single character from the console without echo or buffering
#ifdef __linux__
#include <termios.h>
char getch() {
  struct termios oldt, newt;
  memset(&oldt, 0, sizeof(oldt));
  memset(&newt, 0, sizeof(newt));
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  char c = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return c;
}
#endif

std::string center(std::string str, size_t n) {
  return std::string((n - str.length()) / 2, ' ') + str;
}

void divide_with_steps(const std::string &dividend, const std::string &divisor,
                       std::size_t accuracy);

namespace eval_with_steps {
char *get_numerical_arguments(const char *expression, bool fromLeft,
                              long *signIndexIn, int outputType);

char *simplify_arithmetic_expression(const char *expression_in, int outputType,
                                     size_t accuracy,
                                     std::vector<std::string> &steps,
                                     bool verbose);

size_t get_corresponding_closing_bracket(const char *str, size_t index);
long find_operational_sign(const char *expression, char sign);
size_t get_back_corresponding_bracket(const char *str, size_t index);
}; // namespace eval_with_steps

std::string remove_extra_front_back_brackets(std::string str) {
  while (str.length() >= 2 &&
         algnum::get_matching_brace(str.c_str(), 0) == str.length() - 1) {
    str = str.substr(1, str.length() - 2);
  }
  return str;
}

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
                           algnum::get_matching_brace(numerator, 0) == -1);

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

void print_result(std::string str) {
  auto v = get_printable_result(str);
  std::cout << v[0] << "\n" << v[1] << "\n" << v[2] << "\n";
}

void print_expression(std::vector<std::string> terms,
                      std::vector<std::string> signs, int padding,
                      std::ofstream *file = NULL,
                      std::vector<std::string> *out = NULL,
                      size_t padding_exclude = 0) {
  signs.push_back("");
  std::vector<std::string> expression = {"", "", ""};
  for (size_t i = 0; i < terms.size(); i++) {
    size_t sign_len = signs[i].length();
    if (signs[i] == "\u00d7") {
      sign_len = 1;
    }

    auto v = get_printable_result(terms[i]);
    expression[0] +=
        v[0] + std::string((signs[i] == "^" ? 0 : 2) + sign_len, ' ');
    expression[1] += v[1];
    if (i < terms.size() - 1) {
      if (signs[i] != "^") {
        expression[1] += " " + signs[i] + " ";
      } else {
        expression[1] += signs[i];
      }
    }
    expression[2] +=
        v[2] + std::string((signs[i] == "^" ? 0 : 2) + sign_len, ' ');
  }
  for (auto i = 0; i < expression.size(); ++i) {
    if (i == padding_exclude)
      continue;

    expression[i] = std::string(padding, ' ') + expression[i];
  }
  if (file != NULL) {
    *file << expression[0] << "\n"
          << expression[1] << "\n"
          << expression[2] << "\n";
    return;
  }
  if (out != NULL) {
    out->push_back(expression[0]);
    out->push_back(expression[1]);
    out->push_back(expression[2]);
    return;
  }
  std::cout << expression[0] << "\n"
            << expression[1] << "\n"
            << expression[2] << "\n";
}

void print_eval_expression(std::string expression, int outputType, int padding,
                           std::vector<std::string> *outTerms = NULL,
                           std::vector<std::string> *outSigns = NULL) {
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
          continue;
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

      if (!signs.empty() && signs.back() == "-") {
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
             algnum::get_matching_brace(left, 0) == left.length() - 1) ||
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
             algnum::get_matching_brace(right, 0) == right.length() - 1) ||
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

std::vector<std::string> tokenize(std::string s) {
  // Tokenize on the character ' ', essentially splitting the string into its
  // individual words
  // Also don't split on spaces inside of parentheses

  // Replace all parentheses with with '(' and ')'
  replace_all(s, "[", "(");
  replace_all(s, "]", ")");
  replace_all(s, "{", "(");
  replace_all(s, "}", ")");

  int bracket_count = 0;

  std::vector<std::string> tokens;
  std::string token;
  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == '(') {
      bracket_count++;
    } else if (s[i] == ')') {
      bracket_count--;
    }

    if (bracket_count != 0) {
      token += s[i];
      continue;
    }

    if (s[i] == ' ') {
      tokens.push_back(token);
      token.clear();
      continue;
    }
    token += s[i];
  }
  tokens.push_back(token);
  return tokens;
}

void print_add_whole_steps(std::string l_in, std::string s_in) {
  if (l_in.length() < s_in.length())
    std::swap(l_in, s_in);

  std::string carry_column, answer;

  bool cf = false;
  long p_2 = s_in.length() - 1;
  for (size_t i = l_in.length() - 1; i + 1 > 0; i--) {
    char s = p_2 >= 0 ? s_in[p_2] : '0';
    char a = s + l_in[i] - '0' + cf;
    cf = false;
    if (a > '9') {
      a -= 10;
      cf = true;
    }
    answer.push_back(a);
    if (cf)
      carry_column.push_back('1');
    else
      carry_column.push_back(' ');
    p_2--;
  }

  if (cf)
    answer.push_back('1');

  std::reverse(carry_column.begin(), carry_column.end());
  std::reverse(answer.begin(), answer.end());

  std::cout << " \u001b[35m" << carry_column << "\u001b[0m\n";
  std::cout << "  " << l_in << "\n+ "
            << std::string(l_in.length() - s_in.length(), ' ') << s_in << "\n"
            << std::string(l_in.length() + 2, '-') << "\n"
            << std::string(l_in.length() + 2 - answer.length(), ' ')
            << "\u001b[32m" << answer << "\u001b[0m\n";
}

void print_help(std::string function) {
  if (function == "eval") {
    std::cout << "\nName: eval\n";
    std::cout << "Description: Simplifies an arithmetic expression involving "
                 "the five basic math operations: addition, subtraction, "
                 "multiplication, division, and exponentiation.\n";
    std::cout
        << "C function equivalent:\nchar *\nsimplify_arithmetic_expression "
           "(const char *expression_in, int outputType,\n                      "
           "          size_t accuracy);\n";
    std::cout
        << "C++ function "
           "equivalent:\nstd::string\narithmetica::simplify_arithmetic_"
           "expression(const std::string &expression_in,\n                     "
           "                       int outputType, size_t accuracy);\n";
    std::cout << "Usage: eval <expression>\n\n";
  }
}

namespace arithmetica_factor_polynomial {
std::string factor_polynomial(std::string expr, std::vector<std::string> &steps,
                              bool show_steps);
};

std::string version = "0.4.1";

std::string autorelease = "0";

size_t accuracy = 10;

bool check_for_implicit_eval(std::string &s) {
  // If the input contains only numbers, (), [], {}, +-*/^ and all brackets are correctly opened and closed, automatically eval
  // This is to make it easier for the user to use the program

  std::string allowed = "0123456789.+-*/^()[]{}";
  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == '(') {
      // Check that the bracket has a valid closing bracket
      size_t closing_bracket = eval_with_steps::get_corresponding_closing_bracket(s.c_str(), i);
      if (closing_bracket == std::string::npos) {
        return false;
      }
    }
    if (s[i] == ')') {
      // Check that the bracket has a valid opening bracket
      size_t opening_bracket = eval_with_steps::get_back_corresponding_bracket(s.c_str(), i);
      if (opening_bracket == std::string::npos) {
        return false;
      }
    }
 
    if (allowed.find(s[i]) == std::string::npos) {
      return false;
    }
  }

  s = "eval " + s;

  return true;
}

int arithmetica_tui(int argc, char **argv) {
  using namespace basic_math_operations;
  using namespace arithmetica;

  std::string pi =
      "3."
      "141592653589793238462643383279502884197169399375105820974944592307816406"
      "286208998628034825342117067982148086513282306647093844609550582231725359"
      "408128481117450284102701938521105559644622948954930381964428810975665933"
      "446128475648233786783165271201909145648566923460348610454326648213393607"
      "260249141273724587006606315588174881520920962829254091715364367892590360"
      "011330530548820466521384146951941511609433057270365759591953092186117381"
      "932611793105118548074462379962749567351885752724891227938183011949129833"
      "673362440656643086021394946395224737190702179860943702770539217176293176"
      "752384674818467669405132000568127145263560827785771342757789609173637178"
      "721468440901224953430146549585371050792279689258923542019956112129021960"
      "864034418159813629774771309960518707211349999998372978049951059731732816"
      "096318595024459455346908302642522308253344685035261931188171010003137838"
      "752886587533208381420617177669147303598253490428755468731159562863882353"
      "7875937519577818577805321712268066130019278766111959092164201989";
  std::string inverse_pi =
      "0."
      "318309886183790671537767526745028724068919291480912897495334688117793595"
      "268453070180227605532506171912145685453515916073785823692229157305755934"
      "821463399678458479933874818155146155492793850615377434785792434795323386"
      "724780483447258023664760228445399511431880923780173805347912240978821873"
      "875688171057446199892886800497344695478919221796646193566149812333972925"
      "609398897304375763149573133928482077991748278697219967736198399924885751"
      "170342357716862235037534321093095073976019478920729518667536118604988993"
      "270610654313551006440649555632794332045893496239196331681212033606071996"
      "267823974997665573308870559510140032481355128777699142621760244398752295"
      "362755529475781266136092915956963522624854628139921550049000595519714178"
      "113805593570263050420032635492041849623212481122912406292968178496918382"
      "870423150815112401743053213604434318281514949165445195492570799750310658"
      "781627963544818716509594146657438081399951815315415698694078717965617434"
      "6851280733790233250914118866552625373000522454359423064225199009";

  std::vector<std::string> functions = {
      "eval", "add",  "mul",        "factor", "sin",  "cos",
      "tan",  "asin", "acos",       "atan",   "sqrt", "exp",
      "log",  "fact", "tocontfrac", "gcd",    "lcm"};
  bool from_file = false;
  bool to_file = false;
  std::ifstream input_file;
  std::ofstream output_file;

  std::string printable_version = "arithmetica ";

  if (!autorelease.empty() && autorelease != "0") {
    printable_version += "alpha (" + autorelease + " commit";
    if (autorelease != "1") {
      printable_version += "s";
    }
    printable_version += " after " + version + ")";
  } else {
    printable_version += version;
  }

  bool no_introduction = false;

  if (argc >= 2) {
    if (std::string(argv[1]) == "--version") {
      std::cout << printable_version << "\n";
      return 0;
    }
    if (std::string(argv[1]) == "--get-tag") {
      if (!autorelease.empty() && autorelease != "0") {
        std::cout << version << "-alpha-" << autorelease << "\n";
        return 0;
      }
      std::cout << version << "\n";
      return 0;
    }

    if (std::string(argv[1]) == "--update-bleeding-edge") {
      std::string command;
#ifdef __linux__
      command = "curl -s -H \"Accept: application/vnd.github.v3.raw\" "
                "https://api.github.com/repos/arithmetica-org/arithmetica-tui/"
                "contents/install_bleeding_edge.sh | sudo bash &";
      int n = std::system(command.c_str());
#endif
#ifdef _WIN32
      int n;
      n = std::system(
          "cd %TEMP% && curl -s -H \"Accept: application/vnd.github.v3.raw\" "
          "https://api.github.com/repos/arithmetica-org/arithmetica-tui/"
          "contents/install_bleeding_edge.bat -o install_bleeding_edge.bat && "
          "install_bleeding_edge.bat && exit");
#endif
      std::exit(0);
    }
    if (std::string(argv[1]) == "--update-stable" ||
        std::string(argv[1]) == "--update") {
      int n = std::system(
          "curl -s -H \"Accept: application/vnd.github.v3.raw\" "
          "https://api.github.com/repos/arithmetica-org/arithmetica-tui/"
          "contents/install_stable.sh | sudo bash &");
      std::exit(0);
    }

    for (int i = 0; i < argc; i++) {
      if (std::string(argv[i]) == "--no-introduction") {
        no_introduction = true;
        continue;
      }
      if (std::string(argv[i]) == "-o") {
        if (argc < i + 1) {
          std::cout << "Usage: arithmetica -o [file]\n";
          std::exit(0);
        }

        to_file = true;
        std::string filename = argv[i + 1];
        output_file.open(filename);

        if (!output_file.good()) {
          std::cerr << "Error: There was a problem opening the file \""
                    << filename << "\" for writing.\n";
          std::exit(1);
        }
      }
    }
    if (std::string(argv[1]) == "-i") {
      if (argc < 3) {
        std::cout << "Usage: arithmetica -i [file]\n";
        std::exit(0);
      }
      from_file = true;
      std::string filename = argv[2];
      input_file.open(filename);
      if (!input_file.good()) {
        std::cerr << "Error: File \"" << filename << "\" not found.\n";
        std::exit(1);
      }
    }
  }

  if (argc != 1 && (!from_file) && (!no_introduction)) {
    std::cout << "Usage: arithmetica [--version] [--get-tag] [--update] "
                 "[--update-bleeding-edge] [-i] [-o]\n";
    std::exit(0);
  }

  if (!no_introduction) {
    std::cout << printable_version;

    if (!autorelease.empty() && autorelease != "0") {
      std::cout << "\n"
                << "This version was automatically compiled and released by "
                   "GitHub Actions. Due to its bleeding edge nature, some "
                   "features might be unstable.\n";
    }

    std::cout << "\nhttps://github.com/arithmetica-org/arithmetica-tui\n\n";

    std::cout << "arithmetica supports showing working with steps (disabled "
                 "by default), toggle this by typing \"showsteps\".\n\n";

    std::cout << "To get started, type help.\n";
  }

  bool show_steps = false;
  bool degree_mode = true;
  bool enable_fractions_eval = true;
  bool verbose_eval = false;
  bool numeric_eval = false;

  std::vector<std::string> history;
  std::string prev_result;
  int history_index = -1; // Current history item

  while (true) {
    ++history_index;
    history.push_back("");
    std::string input;
    size_t input_index = 0;
    std::cout << "arithmetica> ";

    if (from_file) {
      if (input_file.eof()) {
        break;
      }

      std::getline(input_file, input);
      std::cout << input << "\n";

      goto after_input;
    }

    char c;
#ifdef __linux__
    while ((c = getch()) != '\n') {
      if (c == 27) { // Escape character (arrow key)
        c = getch();
        if (c == 91) { // Left square bracket
          c = getch();
          if (c == 65) { // Up arrow
            if (history_index != 0) {
              history_index--;
            }
            input = history[history_index];
            input_index = input.length();
            std::cout << "\33[2K\rarithmetica> " << input;
          } else if (c == 66) { // Down arrow
            if (history_index < history.size() - 1) {
              history_index++;
            }
            input = history[history_index];
            input_index = input.length();
            std::cout << "\33[2K\rarithmetica> " << input;
          } else if (c == 67) { // Right arrow
            if (input_index < input.length()) {
              input_index++;
              std::cout << "\rarithmetica> " << input.substr(0, input_index);
            }
          } else if (c == 68) { // Left arrow
            if (input_index != 0) {
              input_index--;
              std::cout << "\rarithmetica> " << input.substr(0, input_index);
            }
          }
        }
      } else if (c == 127 || c == 8 || c == 27) { // Backspace/delete/delete key
        if (input_index != 0) {
          if (c == 127 || c == 8) { // Backspace
            // Remove the character behind [input_index]
            input.erase(input_index - 1, 1);
            input_index--;
          } else { // Delete key
            // Remove the character in front of [input_index]
            input.erase(input_index, 1);
          }
          for (size_t i = 0; i < (input.length() + 13) / get_console_width();
               ++i) {
            std::cout << "\33[2K\r\033[A";
          }
          std::cout << "\33[2K\rarithmetica> " << input;
          std::cout << "\rarithmetica> " << input.substr(0, input_index);
        }
      } else {
        // Add the character to the string in front of [input_index]
        if (input_index == input.length() - 1) {
          input.push_back(c);
        } else {
          input.insert(input_index, 1, c);
        }
        input_index++;
        for (size_t i = 0; i < (input.length() + 11) / get_console_width();
             ++i) {
          std::cout << "\33[2K\r\033[A";
        }
        std::cout << "\33[2K\rarithmetica> " << input;
        std::cout << "\rarithmetica> " << input.substr(0, input_index);
      }
    }
#endif

#ifdef _WIN32
    // Fortunately, windows supports arrow keys and history during std::getline
    // by default Which is amazing, for once windows is better than linux Thank
    // you Microsoft
    std::getline(std::cin, input);
#endif

#ifdef __linux__
    // Don't add consecutive duplicates or empty strings to history
    if (input.empty() ||
        (history.size() >= 2 && history[history.size() - 2] == input)) {
      history.pop_back();
    } else {
      history[history.size() - 1] = input;
    }

    history_index = history.size() - 1;
#endif

#ifdef __linux__
    std::cout << "\n";
#endif

  after_input:

    // remove front and back whitespace
    input.erase(0, input.find_first_not_of(' '));
    input.erase(input.find_last_not_of(' ') + 1);

    if (input.substr(0, 4) == "help") {
      auto tokens = tokenize(input);

      if (tokens.size() == 1) {
        std::cout << "\n";
        std::cout << "help - show this help message\n";
        std::cout << "quickstart - show a quickstart guide\n";
        std::cout
            << "showsteps - toggle showing steps (for supported functions)\n";
        std::cout << "eval <expression> - evaluate an arithmetic expression\n";
        std::cout << "add <number/algexpr> <number/algexpr> - add two numbers "
                     "or algebraic expressions\n";
        std::cout << "mul <number/algexpr> <number/algexpr> - multiply two "
                     "numbers or algebraic expressions\n";
        std::cout << "factor <polynomial> - factor a polynomial\n";
        std::cout << "sin/cos/tan <angle> - trigonometric functions\n";
        std::cout << "asin/acos/atan <number> - inverse trigonometric "
                     "functions\n";
        std::cout << "sqrt <number> - square root\n";
        std::cout << "exp <number> - compute e^<number>\n";
        std::cout
            << "log <base> <number> - compute log_<base>(<number>), or ln "
               "(<number>) if <base> is not specified\n";
        std::cout << "fact <number> - computes the factorial of <number>\n";
        std::cout << "tocontfrac <number> - converts <number> to a continued "
                     "fraction\n";
        std::cout << "gcd <number> <number> ... - computes the greatest common "
                     "divisor of the given numbers\n";
        std::cout << "lcm <number> <number> ... - computes the least common "
                     "multiple of the given numbers\n";
        std::cout
            << "\nFor help with a specific function, type help <function>\n\n";
        std::cout << "Options:\n";
        std::cout << "  showsteps - " << (show_steps ? "true" : "false")
                  << "\n";
        std::cout << "  degreemode - " << (degree_mode ? "enabled" : "disabled")
                  << "\n";
        std::cout << "  fractionseval (enable fractions during eval) - "
                  << (enable_fractions_eval ? "enabled" : "disabled") << "\n";
        std::cout << "  verboseeval (include verbose explanations with steps "
                     "during eval) - "
                  << (verbose_eval ? "enabled" : "disabled") << "\n";
        std::cout << "  numericeval (purely numeric evaluation during eval) - "
                  << (numeric_eval ? "enabled" : "disabled") << "\n";
        std::cout << "  accuracy - " << accuracy
                  << ", change with accuracy <num>\n\n";
        continue;
      }

      std::string function = tokens[1];

      bool found = false;
      for (auto &i : functions) {
        if (i == tokens[1].substr(0, i.length())) {
          found = true;
          function = i;
          break;
        }
      }

      if (!found) {
        std::cout << "No help entry found for " << tokens[1] << "\n";
        continue;
      }

      print_help(function);
    }

    if (input == "exit" || input == "quit") {
      break;
    }
    if (input.substr(0, 3) == "gcd") {
      auto tokens = tokenize(input);
      if (tokens.size() < 2) {
        std::cout << "Usage: gcd <number> <number> ...\n";
        continue;
      }

      std::string gcd = tokens[1];
      for (int i = 2; i < tokens.size(); i++) {
        gcd = arithmetica::igcd(gcd, tokens[i]);
      }

      std::cout << "==> " << gcd << "\n";
    }
    if (input.substr(0, 3) == "lcm") {
      auto tokens = tokenize(input);
      if (tokens.size() < 2) {
        std::cout << "Usage: lcm <number> <number> ...\n";
        continue;
      }

      std::string lcm = tokens[1];
      for (int i = 2; i < tokens.size(); i++) {
        lcm = arithmetica::ilcm(lcm, tokens[i]);
      }

      std::cout << "==> " << lcm << "\n";
    }
    if (input.substr(0, 10) == "tocontfrac") {
      auto tokens = tokenize(input);
      if (tokens.size() != 2) {
        std::cout << "Usage: tocontfrac <number>\n";
        continue;
      }

      arithmetica::Fraction f = tokens[1];
      auto answer = arithmetica::fraction_to_continued_fraction(f.numerator,
                                                                f.denominator);
      std::cout << "==> [" << answer[0] << "; ";
      for (int i = 1; i < answer.size(); i++) {
        std::cout << answer[i];
        if (i != answer.size() - 1) {
          std::cout << ", ";
        }
      }
      std::cout << "]\n";
    }
    if (tokenize(input)[0] == "fact" || tokenize(input)[0] == "factorial") {
      auto tokens = tokenize(input);
      if (tokens.size() != 2) {
        std::cout << "Usage: fact <number>\n";
        continue;
      }

      if (tokens[1].find('.') != std::string::npos) {
        std::cout << "Invalid argument: " << tokens[1] << "\n";
        continue;
      }

      unsigned long long n;
      try {
        n = std::stoull(tokens[1]);
      } catch (std::invalid_argument &e) {
        std::cout << "Invalid argument: " << tokens[1] << "\n";
        continue;
      } catch (std::out_of_range &e) {
        std::cout << "Number too large (for now): " << tokens[1] << "\n";
        continue;
      }

      if (n == 0) {
        std::cout << "==> 1\n";
        continue;
      }

      std::cout << "==> " << arithmetica::factorial(n) << "\n";
    }
    if (input.substr(0, 3) == "log") {
      auto tokens = tokenize(input);
      if (tokens.size() != 2 && tokens.size() != 3) {
        std::cout << "Usage: log <base> <number>\n";
        continue;
      }

      if (tokens.size() == 2) {
        std::cout << "==> "
                  << arithmetica::natural_logarithm(tokens[1], accuracy)
                  << "\n";
      } else {
        std::cout
            << "==> "
            << round_decimal(
                   basic_math_operations::divide(
                       arithmetica::natural_logarithm(tokens[2], accuracy + 3),
                       arithmetica::natural_logarithm(tokens[1], accuracy + 3),
                       accuracy),
                   accuracy)
            << "\n";
      }
    }
    if (input.substr(0, 3) == "exp") {
      auto tokens = tokenize(input);
      if (tokens.size() != 2) {
        std::cout << "Usage: exp <number>\n";
        continue;
      }
      std::cout << "==> " << arithmetica::exponential(tokens[1], accuracy)
                << "\n";
    }
    if (input.substr(0, 4) == "sqrt") {
      auto tokens = tokenize(input);
      if (tokens.size() != 2) {
        std::cout << "Usage: sqrt <number>\n";
        continue;
      }
      std::string ans = arithmetica::square_root(tokens[1], accuracy);
      if (to_file) {
        output_file << ans << "\n";
      }
      std::cout << "==> " << ans << "\n";
    }
    if (input == "numericeval") {
      numeric_eval = !numeric_eval;
      std::cout << "numeric evaluation is now "
                << (numeric_eval ? "enabled" : "disabled") << "\n";
    }
    if (input == "verboseeval") {
      verbose_eval = !verbose_eval;
      std::cout << "verbose evaluation is now "
                << (verbose_eval ? "enabled" : "disabled") << "\n";
    }
    if (input == "fractionseval") {
      enable_fractions_eval = !enable_fractions_eval;
      std::cout << "fractions are now "
                << (enable_fractions_eval ? "displayed" : "not disabled")
                << " in eval\n";
      continue;
    }
    if (input.substr(0, 4) == "asin" || input.substr(0, 6) == "arcsin" ||
        input.substr(0, 4) == "acos" || input.substr(0, 6) == "arccos" ||
        input.substr(0, 4) == "atan" || input.substr(0, 6) == "arctan") {
      auto tokens = tokenize(input);

      bool is_acos = tokens[0] == "acos" || tokens[0] == "arccos";
      bool is_atan = tokens[0] == "atan" || tokens[0] == "arctan";

      std::vector<std::string> functions = {"arcsin", "arccos", "arctan"};
      std::vector<std::string> example_usages = {
          "Example usage: arcsin 0.5 ==> 30\u00b0 = 0.523598 rad",
          "Example usage: arccos 0.5 ==> 60\u00b0 = 1.0472 rad",
          "Example usage: arctan 1 ==> 45\u00b0 = 0.785398 rad"};

      if (tokens.size() == 1) {
        std::cout << "Example usage: " << example_usages[is_acos + 2 * is_atan]
                  << "\n";
        continue;
      }
      std::string num = tokens[1];
      std::cout << " ==> " << functions[is_acos + 2 * is_atan] << "(" << num
                << ") = ";
      std::string answer;

      if (is_acos) {
        answer = arithmetica::arccos(num, accuracy + 3);
      } else if (is_atan) {
        answer = arithmetica::arctan(num, accuracy + 3);
      } else {
        answer = arithmetica::arcsin(num, accuracy + 3);
      }

      std::string rad_ans = round_decimal(answer, accuracy);
      if (to_file) {
        output_file << rad_ans << "\n";
      }
      std::cout << round_decimal(
                       basic_math_operations::multiply(
                           basic_math_operations::multiply(answer, "180"),
                           inverse_pi.substr(0, 6 + accuracy)),
                       accuracy)
                << "\u00b0 = " << rad_ans << " rad\n";
      prev_result = rad_ans;
    }
    if (input.substr(0, 3) == "sin" || input.substr(0, 3) == "cos" ||
        input.substr(0, 3) == "tan") {
      bool is_cos = input.substr(0, 3) == "cos";
      bool is_tan = input.substr(0, 3) == "tan";

      std::vector<std::string> function_names = {"sin", "cos", "tan"};
      std::vector<std::string> example_usages = {
          "sin 30 ==> 0.5", "cos 60 ==> 0.5", "tan 45 ==> 1"};

      auto tokens = tokenize(input);
      if (tokens.size() == 1) {
        std::cout << "Example usage: " << example_usages[is_cos + is_tan * 2]
                  << ", assuming that degree mode is enabled\n";
        continue;
      }
      std::string num = tokens[1];
      std::cout << " ==> " << function_names[is_cos + is_tan * 2] << "(" << num
                << (degree_mode ? "\u00b0" : " rad") << ") = ";
      if (degree_mode) {
        num = basic_math_operations::divide(
            basic_math_operations::multiply(num, pi.substr(0, 6 + accuracy)),
            "180", accuracy + 3);
      }

      std::string answer;
      if (is_cos) {
        answer = arithmetica::cosine(num, accuracy + 3);
      } else if (is_tan) {
        answer = arithmetica::tangent(num, accuracy + 3);
      } else {
        answer = arithmetica::sine(num, accuracy + 3);
      }

      std::cout << round_decimal(answer, accuracy) << "\n";
    }
    if (input.substr(0, 8) == "accuracy") {
      auto tokens = tokenize(input);
      if (tokens.size() == 1) {
        std::cout << "accuracy is currently " << accuracy << "\n";
        continue;
      }
      std::string &num = tokens[1];
      if (num.length() > 18) {
        std::cout << "Number too large\n";
        continue;
      }
      if (num[0] == '-') {
        std::cout << "Number must be positive\n";
        continue;
      }
      try {
        accuracy = std::stoull(num);
      } catch (std::invalid_argument &e) {
        std::cout << "Invalid number: " << num << "\n";
        continue;
      }
      std::cout << "accuracy is now " << accuracy << "\n";
    }

    if (tokenize(input)[0] == "factor" || tokenize(input)[0] == "factorize") {
      if (input.length() < 8) {
        std::cout << "Example usage: factor x^2+3x+2 => (x+1)(x+2)\n";
        continue;
      }
      auto tokens = tokenize(input);
      std::string expression;
      for (size_t i = 1; i < tokens.size(); ++i) {
        expression += tokens[i];
      }
      std::vector<std::string> steps;
      std::string factored = arithmetica_factor_polynomial::factor_polynomial(
          expression, steps, show_steps);
      if (to_file) {
        output_file << factored << "\n";
      }
      std::cout << "\n";
      if (factored != "ERROR") {
        steps.push_back(factored);
        for (auto &i : steps) {
          replace_all(i, "+", " + ");
          replace_all(i, "-", " - ");
          std::cout << "==> " << i << "\n";
        }
      }
      std::cout << "\n";
    }

    if (input == "showsteps") {
      show_steps = !show_steps;
      std::cout << "showsteps is now " << (show_steps ? "true" : "false")
                << "\n";
    }
    if (input == "degreemode") {
      degree_mode = !degree_mode;
      std::cout << "degreemode is now "
                << (degree_mode ? "enabled" : "disabled") << "\n";
    }
    if (input.substr(0, 4) == "eval" || check_for_implicit_eval(input)) {
      if (input.length() < 6) {
        std::cout << "Example usage: eval 2+2 => 4\n";
        continue;
      }
      auto tokens = tokenize(input);
      std::string expression = tokens[1];
      for (auto i = 2; i < tokens.size(); ++i) {
        expression += tokens[i];
      }

      if (numeric_eval) {
        std::string ans = arithmetica::simplify_arithmetic_expression(
            expression, 0, accuracy);
        if (to_file) {
          output_file << ans << "\n";
        }
        std::cout << " ==> " << ans << "\n";
        continue;
      }

      if (!show_steps) {
        std::string result = arithmetica::simplify_arithmetic_expression(
            expression, 1, accuracy);
        std::vector<std::string> to_print = {
            arithmetica::simplify_arithmetic_expression(result, 0, accuracy)};
        if (enable_fractions_eval) {
          to_print.push_back(result);
          to_print.push_back(
              arithmetica::simplify_arithmetic_expression(result, 2, 0));
        }
        // remove duplicates from to_print
        to_print.erase(std::unique(to_print.begin(), to_print.end()),
                       to_print.end());
        if (to_file) {
          std::string s;
          for (auto &i : to_print) {
            s += i + ", ";
          }
          if (s.length() > 2) {
            s = s.substr(0, s.length() - 2);
          }
          output_file << s << "\n";
        }
        print_expression(to_print,
                         std::vector<std::string>(to_print.size() - 1, "="), 0);
      } else {
        std::vector<std::string> steps;
        char *answer = eval_with_steps::simplify_arithmetic_expression(
            expression.c_str(), 1, accuracy, steps, verbose_eval);
        std::string answer_cpp = answer;
        free(answer);
        // Remove consecutive duplicates from steps
        // if arr[i] == arr[i+1], then remove arr[i]
        for (size_t i = 0; i < steps.size() - 1; ++i) {
          int x = compare_printed_text(steps[i], steps[i + 1]);

          if (x == 1) {
            steps.erase(steps.begin() + i + 1);
            --i;
          } else if (x == 2) {
            steps.erase(steps.begin() + i);
            --i;
          }
        }

        bool print_original = true;
        for (auto &i : steps) {
          if (compare_text_without_ansi(i, "==> " + expression)) {
            print_original = false;
            break;
          }
        }

        replace_all(expression, "+", " + ");
        replace_all(expression, "-", " - ");
        replace_all(expression, "( - ", "(-");
        replace_all(expression, "^ - ", "^-");
        replace_all(expression, "*", " \u00d7 ");
        std::cout << "\n";

        if (verbose_eval) {
          std::cout << "Task: Simplify " << expression << "\n\n";
        } else if (print_original) {
          std::cout << "==> " << expression << "\n";
        }

        std::string s;
        for (auto &i : steps) {
          // replace_all(i, "+", " + ");
          // replace_all(i, "-", " - ");
          // replace_all(i, "( - ", "(-");
          // replace_all(i, "^ - ", "^-");
          // replace_all(i, "*", " \u00d7 ");
          s += i + "\n";
          print_eval_expression(i, 1, 0);
        }
        // std::cout << s;
        std::string decimal_version =
            arithmetica::simplify_arithmetic_expression(answer_cpp, 0,
                                                        accuracy);
        if (decimal_version != answer_cpp) {
          std::vector<std::string> to_print = {
              decimal_version, answer_cpp,
              arithmetica::simplify_arithmetic_expression(answer_cpp, 2,
                                                          accuracy)};
          to_print.erase(std::unique(to_print.begin(), to_print.end()),
                         to_print.end());
          std::cout << "[";
          for (auto i = 0; i < to_print.size(); ++i) {
            to_print[i].erase(
                std::remove(to_print[i].begin(), to_print[i].end(), ' '),
                to_print[i].end());
            std::cout << to_print[i];
            if (i != to_print.size() - 1) {
              std::cout << ", ";
            }
          }
          std::cout << "]\n";
        }
        if (s.length() > 1 && s[s.length() - 2] != '\n') {
          std::cout << "\n";
        }
      }
    }
    if (input.substr(0, 3) == "add") {
      std::vector<std::string> tokens = tokenize(input);
      if (tokens.size() < 3) {
        std::cout << "Example usage: add 1/2 1/2 => 1\n";
        continue;
      }

      for (std::size_t i = 3; i < tokens.size(); ++i) {
        tokens[2] += "+" + tokens[i];
      }

      if (tokens[1].find_first_of(
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") !=
              std::string::npos ||
          tokens[2].find_first_of(
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") !=
              std::string::npos) {
        algnum::algexpr a = algnum::algexpr(tokens[1].c_str());
        algnum::algexpr b = algnum::algexpr(tokens[2].c_str());

        std::string expr = (a + b).latex();
        // Remove spaces
        expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());
        // Add spaces around operators
        replace_all(expr, "+", " + ");
        replace_all(expr, "-", " - ");

        std::cout << "==> " << expr << "\n";
        continue;
      }

      if (!show_steps) {
        arithmetica::Fraction f_1 = arithmetica::Fraction(tokens[1]);
        arithmetica::Fraction f_2 = arithmetica::Fraction(tokens[2]);
        std::string expr = arithmetica::to_string((f_1 + f_2));
        std::vector<std::string> answers = {
            simplify_arithmetic_expression(expr, 0, 10), expr,
            simplify_arithmetic_expression(expr, 2, 0)};
        // remove duplicates from answers
        answers.erase(std::unique(answers.begin(), answers.end()),
                      answers.end());
        print_expression(answers,
                         std::vector<std::string>(answers.size() - 1, "="), 0);
      } else {
        if (tokens[1].find('/') != std::string::npos ||
            tokens[2].find('/') != std::string::npos) {
          std::cout << "Task:    ";
          print_expression({tokens[1], tokens[2]}, {"+", "+"}, 9);

          arithmetica::Fraction f_1 = arithmetica::Fraction(tokens[1]);
          arithmetica::Fraction f_2 = arithmetica::Fraction(tokens[2]);
          int step = 0;

          if (f_1.numerator != tokens[1].substr(0, tokens[1].find('/')) ||
              f_2.numerator != tokens[2].substr(0, tokens[2].find('/'))) {
            ++step;
            std::cout << "\nStep #" << step
                      << ": Simplify the input fractions\n";
            std::cout << "    " << tokens[1] << " ==> "
                      << arithmetica::to_string(f_1) << "\n";
            std::cout << "    " << tokens[2] << " ==> "
                      << arithmetica::to_string(f_2) << "\n";
            tokens[1] = arithmetica::to_string(f_1);
            tokens[2] = arithmetica::to_string(f_2);
          }

          if (f_1.denominator != f_2.denominator) {
            std::string lcm =
                arithmetica::ilcm(f_1.denominator, f_2.denominator);
            ++step;
            std::cout
                << "\nStep #" << step
                << ": Find the lowest common multiple of the denominators.\n";
            std::cout << "    ==> lcm(" << f_1.denominator << ", "
                      << f_2.denominator << ") = " << lcm << "\n";
            ++step;
            std::cout << "\nStep #" << step
                      << ": Multiply each of the fractions by their lowest "
                         "common multiple over their denominator.\n";
            std::cout << "    ";
            print_expression({tokens[1], tokens[2]}, {"+", "+"}, 4);
            std::cout << "==> ";
            std::string lcm_den_1 = divide(lcm, f_1.denominator, 0);
            std::string lcm_den_2 = divide(lcm, f_2.denominator, 0);
            print_expression({lcm_den_1 + "/" + lcm_den_1, tokens[1],
                              lcm_den_2 + "/" + lcm_den_2, tokens[2]},
                             {"x", "+", "x"}, 4);
            std::cout << "==> ";
            print_expression({multiply(f_1.numerator, lcm_den_1) + "/" +
                                  multiply(f_1.denominator, lcm_den_1),
                              multiply(f_2.numerator, lcm_den_2) + "/" +
                                  multiply(f_2.denominator, lcm_den_2)},
                             {"+", "+"}, 4);
          }

          ++step;
          std::cout << "\nStep #" << step
                    << ": Since both fractions have the same denominator, we "
                       "can simply add their numerators.\n";
          std::cout << "==> ";
          print_expression({arithmetica::to_string(f_1 + f_2)}, {""}, 4);

          continue;
        }

        // add_whole
        if (tokens[1].find('.') == std::string::npos &&
            tokens[2].find('.') == std::string::npos) {
          print_add_whole_steps(tokens[1], tokens[2]);
          continue;
        }

        std::cout << "Sorry, this isn't supported yet!\n";
      }
    }
    if (input.substr(0, 3) == "mul") {
      std::vector<std::string> tokens = tokenize(input);
      if (tokens.size() < 3) {
        std::cout << "Example usage: mul 1/2 1/2 => 1/4\n";
        continue;
      }

      for (std::size_t i = 3; i < tokens.size(); ++i) {
        if (!(tokens[2].back() == '+' || '-' || '*' || '/')) {
          tokens[2] += "*";
        }
        tokens[2] += tokens[i];
      }

      std::cout << "tokens[1] = " << tokens[1] << "\n";
      std::cout << "tokens[2] = " << tokens[2] << "\n";

      for (std::string &i : tokens) {
        if (i == "$PREV_RESULT") {
          i = prev_result;
        }
      }

      // Check if the input (tokens[1] or tokens[2]) contains letters ('a' - 'z'
      // or 'A' - 'Z'), if so, perform algebraic multiplication
      if (tokens[1].find_first_of(
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") !=
              std::string::npos ||
          tokens[2].find_first_of(
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") !=
              std::string::npos) {
        algnum::algexpr expr_1 = algnum::algexpr(tokens[1].c_str());
        algnum::algexpr expr_2 = algnum::algexpr(tokens[2].c_str());
        std::string answer = (expr_1 * expr_2).latex();
        // Remove all spaces from the answer
        answer.erase(std::remove(answer.begin(), answer.end(), ' '),
                     answer.end());
        replace_all(answer, "+", " + ");
        replace_all(answer, "-", " - ");
        if (to_file) {
          output_file << answer << "\n";
        }
        std::cout << "==> " << answer << "\n";
        continue;
      }

      if (!show_steps) {
        if (tokens[1].find('/') == std::string::npos &&
            tokens[2].find('/') == std::string::npos) {
          std::string ans =
              basic_math_operations::multiply(tokens[1], tokens[2]);
          std::cout << "==> " << ans << "\n";
          if (to_file) {
            output_file << ans << "\n";
          }
        }
      } else {
        if (tokens[1].find('/') != std::string::npos ||
            tokens[2].find('/') != std::string::npos) {
          std::cout << "Task:    ";
          print_expression({tokens[1], tokens[2]}, {"x", "x"}, 9);

          arithmetica::Fraction f_1 = arithmetica::Fraction(tokens[1]);
          arithmetica::Fraction f_2 = arithmetica::Fraction(tokens[2]);
          int step = 0;

          if (f_1.numerator != tokens[1].substr(0, tokens[1].find('/')) ||
              f_2.numerator != tokens[2].substr(0, tokens[2].find('/'))) {
            ++step;
            std::cout << "\nStep #" << step
                      << ": Simplify the input fractions\n";
            std::cout << "    " << tokens[1] << " ==> "
                      << arithmetica::to_string(f_1) << "\n";
            std::cout << "    " << tokens[2] << " ==> "
                      << arithmetica::to_string(f_2) << "\n";
            tokens[1] = arithmetica::to_string(f_1);
            tokens[2] = arithmetica::to_string(f_2);
          }

          ++step;
          std::cout << "\nStep #" << step
                    << ": Multiply the numerators and denominators of the "
                       "fractions.\n";
          std::cout << "    ";
          print_expression({tokens[1], tokens[2]}, {"x", "x"}, 4);
          std::cout << "==> ";
          print_expression({multiply(f_1.numerator, f_2.numerator) + "/" +
                            multiply(f_1.denominator, f_2.denominator)},
                           {""}, 4);

          arithmetica::Fraction result = f_1 * f_2;
          if (result.numerator != multiply(f_1.numerator, f_2.numerator) ||
              result.denominator !=
                  multiply(f_1.denominator, f_2.denominator)) {
            ++step;
            std::cout << "\nStep #" << step << ": Simplify the result.\n";
            std::cout << "==> ";
            print_expression({arithmetica::to_string(result)}, {""}, 4);
          }

          continue;
        }

        // mul_whole
        if (tokens[1].find('.') == std::string::npos &&
            tokens[2].find('.') == std::string::npos) {
          // If the numbers are 1 digit, the result is trivial
          if (tokens[1].length() == 1 && tokens[2].length() == 1) {
            std::cout << "==> " << std::stoi(tokens[1]) * std::stoi(tokens[2])
                      << "\n";
            continue;
          }

          std::string s_in = tokens[1];
          std::string l_in = tokens[2];
          if (l_in.length() < s_in.length()) {
            std::swap(s_in, l_in);
          }

          std::vector<std::string> to_add;
          size_t max_len = 0;
          std::string result = "0";
          for (auto i = 0; i < s_in.length(); ++i) {
            to_add.push_back(
                multiply(l_in, std::string(1, s_in[s_in.length() - i - 1])));
            if (i != 0) {
              to_add[i] += std::string(i, '0');
            }
            max_len = std::max(max_len, to_add[i].length());
            result = add(result, to_add[i]);
          }

          size_t m = result.length();

          std::cout << std::string(m - l_in.length() + 2, ' ') << l_in << "\n";
          std::cout << " x" << std::string(m - s_in.length(), ' ') << s_in
                    << "\n";
          std::cout << std::string(m - l_in.length(), ' ')
                    << std::string(l_in.length() + 2, '-') << "\n";
          for (auto i = 0; i < to_add.size(); ++i) {
            std::string print =
                std::string(m - to_add[i].length() + 2, ' ') + to_add[i];
            if (i != 0)
              print[0] = '+';
            std::cout << print << "\n";
          }
          std::cout << "  " << std::string(result.length(), '-') << "\n";
          std::cout << "  " << result << "\n";
          continue;
        }

        std::cout << "Sorry, this isn't supported yet!\n";
      }
    }
    if (input.substr(0, 3) == "div") {
      auto tokens = tokenize(input);
      if (tokens.size() < 3) {
        std::cout << "Invalid input!\n";
        continue;
      }
      if (show_steps) {
        divide_with_steps(tokens[1], tokens[2], accuracy);
      } else {
        std::cout << "==> " << divide(tokens[1], tokens[2], accuracy) << "\n";
      }
    }
  }

  if (from_file) {
    input_file.close();
  }
  if (to_file) {
    output_file.close();
  }

  return 0;
}