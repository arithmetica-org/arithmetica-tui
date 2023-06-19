#include "algnum.hpp"
#include <algorithm>
#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "constants.hpp"
#include <functions.hpp>
#include <helpers.hpp>

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

namespace arithmetica_factor_polynomial {
std::string factor_polynomial(std::string expr, std::vector<std::string> &steps,
                              bool show_steps);
};

int arithmetica_tui(int argc, char **argv) {
  using namespace basic_math_operations;
  using namespace arithmetica;

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
  bool reprint_input = false;

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
      if (std::string(argv[i]) == "--reprint-input") {
        reprint_input = true;
        continue;
      }
    }
  }

  if (argc != 1 && (!no_introduction)) {
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
  bool experimental_pretty_fractions_eval = true;

  std::vector<std::string> history;
  std::string prev_result;
  int history_index = -1; // Current history item

  while (true) {
    ++history_index;
    history.push_back("");
    std::string input;
    size_t input_index = 0;
    std::cout << "arithmetica> ";

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

    if (reprint_input) {
      std::cout << "\rarithmetica> " << input << "\n";
    }
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

    // remove front and back whitespace
    input.erase(0, input.find_first_not_of(' '));
    input.erase(input.find_last_not_of(' ') + 1);

    auto tokens = tokenize(input);

    if (input.substr(0, 4) == "help") {
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
        std::cout
            << "  experimentalprettyfractionseval (pretty print fractions "
               "during eval) - "
            << (experimental_pretty_fractions_eval ? "enabled" : "disabled")
            << "\n";
        std::cout << "  accuracy - " << accuracy
                  << ", change with accuracy <num>\n\n";
        continue;
      }
    }

    if (input == "exit" || input == "quit") {
      break;
    }

    if (tokens[0] == "experimentalprettyfractionseval") {
      if (tokens.size() == 1) {
        experimental_pretty_fractions_eval =
            !experimental_pretty_fractions_eval;
        std::cout << "experimentalprettyfractionseval is now "
                  << (experimental_pretty_fractions_eval ? "enabled"
                                                         : "disabled")
                  << "\n";
        continue;
      }

      if (tokens[1] == "true") {
        experimental_pretty_fractions_eval = true;
      } else if (tokens[1] == "false") {
        experimental_pretty_fractions_eval = false;
      } else {
        std::cout << "Usage: experimentalprettyfractionseval <true/false>\n";
      }
      continue;
    }

    if (input.substr(0, 3) == "gcd") {

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
    if (tokens[0] == "exp" || tokens[0] == "exponential") {
      if (tokens.size() != 2) {
        std::cout << "Usage: exp <number>\n";
        continue;
      }
      std::cout << "==> " << arithmetica::exponential(tokens[1], accuracy)
                << "\n";
    }
    if (input.substr(0, 4) == "sqrt") {

      if (tokens.size() != 2) {
        std::cout << "Usage: sqrt <number>\n";
        continue;
      }
      std::string ans = arithmetica::square_root(tokens[1], accuracy);
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

      std::string expression;
      for (size_t i = 1; i < tokens.size(); ++i) {
        expression += tokens[i];
      }
      std::vector<std::string> steps;
      std::string factored = arithmetica_factor_polynomial::factor_polynomial(
          expression, steps, show_steps);
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
      tokens = tokenize(input);
      if (input.length() < 6) {
        std::cout << "Example usage: eval 2+2 => 4\n";
        continue;
      }

      std::string expression = tokens[1];
      for (auto i = 2; i < tokens.size(); ++i) {
        expression += tokens[i];
      }

      if (numeric_eval) {
        std::string ans = arithmetica::simplify_arithmetic_expression(
            expression, 0, accuracy);
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
          if (experimental_pretty_fractions_eval) {
            print_eval_expression(i, 1, 0);
          }
        }
        if (!experimental_pretty_fractions_eval) {
          std::cout << s;
        }
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
        std::cout << "==> " << answer << "\n";
        continue;
      }

      if (!show_steps) {
        if (tokens[1].find('/') == std::string::npos &&
            tokens[2].find('/') == std::string::npos) {
          std::string ans =
              basic_math_operations::multiply(tokens[1], tokens[2]);
          std::cout << "==> " << ans << "\n";
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

  return 0;
}