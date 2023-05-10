#include <algorithm>
#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <vector>

// Get a single character from the console without echo or buffering
char getch() {
  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  char c = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return c;
}

std::string center(std::string str, size_t n) {
  return std::string((n - str.length()) / 2, ' ') + str;
}

std::vector<std::string> get_printable_result(std::string str) {
  std::string whole, numerator, denominator;
  if (str.find('/') != std::string::npos) {
    if (str.find(' ') == std::string::npos) {
      numerator = str.substr(0, str.find('/'));
      denominator = str.substr(str.find('/') + 1, str.length());
    } else {
      whole = str.substr(0, str.find(' '));
      numerator =
          str.substr(str.find(' ') + 3, str.find('/') - str.find(' ') - 3);
      denominator = str.substr(str.find('/') + 1, str.length());
    }
  } else {
    return {std::string(str.length(), ' '), str,
            std::string(str.length(), ' ')};
  }

  //   1
  // 3 -
  //   2

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

  return answer;
}

void print_result(std::string str) {
  auto v = get_printable_result(str);
  std::cout << v[0] << "\n" << v[1] << "\n" << v[2] << "\n";
}

void print_expression(std::vector<std::string> terms,
                      std::vector<std::string> signs, int padding) {
  signs.push_back("");
  std::vector<std::string> expression = {"", "", ""};
  for (size_t i = 0; i < terms.size(); i++) {
    auto v = get_printable_result(terms[i]);
    expression[0] += v[0] + std::string(2 + signs[i].length(), ' ');
    expression[1] += v[1];
    if (i < terms.size() - 1)
      expression[1] += " " + signs[i] + " ";
    expression[2] += v[2] + std::string(2 + signs[i].length(), ' ');
  }
  for (auto i = 1; i < expression.size(); ++i) {
    expression[i] = std::string(padding, ' ') + expression[i];
  }
  std::cout << expression[0] << "\n"
            << expression[1] << "\n"
            << expression[2] << "\n";
}

std::vector<std::string> tokenize(std::string s) {
  // Tokenize on the character ' ', essentially splitting the string into its
  // individual words
  std::vector<std::string> tokens;
  std::string token;
  for (size_t i = 0; i < s.length(); i++) {
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
std::string factor_polynomial(std::string expr, std::vector<std::string> &steps, bool show_steps);
};

std::string version = "0.1.2";

int main(int argc, char **argv) {
  using namespace basic_math_operations;
  using namespace arithmetica;

  std::vector<std::string> functions = {"eval", "add", "mul", "factor"};

  //   std::cout << "Welcome to arithmetica, the command line wrapper for the "
  //                "arithmetica library! ";
  //   std::cout << "Whether it's basic arithmetic, fraction addition, or "
  //                "evaluating trigonometric functions to hundreds of decimal "
  //                "places, arithmetica has you covered!\n\n";

  std::cout << "arithmetica " << version
            << "\nhttps://github.com/avighnac/arithmetica-tui\n\n";

  std::cout << "arithmetica supports showing working with steps (disabled "
               "by default), toggle this by typing \"showsteps\".\n\n";

  std::cout << "To get started, type help.\n";
  std::cout << "If you don't like reading helps, type quickstart instead.\n";

  bool show_steps = false;

  std::vector<std::string> history;
  int history_index = -1; // Current history item

  while (true) {

    ++history_index;
    history.push_back("");
    std::string input;
    size_t input_index = 0;
    std::cout << "arithmetica> ";
    char c;
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
      } else if (c == 127 || c == 8) { // Backspace/delete
        if (input_index != 0) {
          // Remove the character behind [input_index]
          input.erase(input_index - 1, 1);
          input_index--;
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
        std::cout << "\33[2K\rarithmetica> " << input;
        std::cout << "\rarithmetica> " << input.substr(0, input_index);
      }
    }

    // Don't add consecutive duplicates or empty strings to history
    if (input.empty() ||
        (history.size() >= 2 && history[history.size() - 2] == input)) {
      history.pop_back();
    } else {
      history[history.size() - 1] = input;
    }

    history_index = history.size() - 1;

    std::cout << "\n";

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
        std::cout << "add <number> <number> - add two numbers\n";
        std::cout << "mul <number> <number> - multiply two numbers\n";
        std::cout << "factor <polynomial> - factor a polynomial\n";
        std::cout
            << "\nFor help with a specific function, type help <function>\n\n";
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

    if (input.substr(0, 6) == "factor") {
      if (input.length() < 8) {
        std::cout << "Example usage: factor x^2+3x+2 => (x+1)(x+2)\n";
        continue;
      }
      std::string expression = input.substr(7);
      std::vector<std::string> steps;
      std::string factored =
          arithmetica_factor_polynomial::factor_polynomial(expression, steps, show_steps);
      if (factored != "ERROR") {
        for (auto &i : steps) {
          std::cout << "==> " << i << "\n";
        }
        std::cout << "==> " << factored << "\n";
      }
    }

    if (input == "showsteps") {
      show_steps = !show_steps;
      std::cout << "showsteps is now " << (show_steps ? "true" : "false")
                << "\n";
    }
    if (input.substr(0, 4) == "eval") {
      if (input.length() < 6) {
        std::cout << "Example usage: eval 2+2 => 4\n";
        continue;
      }
      std::string expression = input.substr(5);
      std::string result =
          arithmetica::simplify_arithmetic_expression(expression, 1, 0);
      print_expression(
          {arithmetica::simplify_arithmetic_expression(result, 0, 10), result,
           arithmetica::simplify_arithmetic_expression(result, 2, 0)},
          {"=", "="}, 0);
    }
    if (input.substr(0, 3) == "add") {
      std::vector<std::string> tokens = tokenize(input);
      if (tokens.size() < 3) {
        std::cout << "Example usage: add 1/2 1/2 => 1\n";
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
        std::vector<std::string> signs;
        for (size_t i = 1; i < answers.size(); ++i) {
          signs.push_back("=");
        }
        print_expression(answers, signs, 0);
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
      if (!show_steps) {
        std::cout << "nah bro\n";
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
      // https://github.com/avighnac/math-new/blob/main/basic_math_operations/Division%20Algorithm/divide.hpp
    }
  }
}
