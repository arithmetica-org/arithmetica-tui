#include "algnum.hpp"
#include <algorithm>
#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>

#include "constants.hpp"
#include <functions.hpp>
#include <helpers.hpp>

std::string remove_spaces(std::string s) {
  std::string ans = s;
  replace_all(ans, " ", "");
  return ans;
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

  outstream << " \u001b[35m" << carry_column << "\u001b[0m\n";
  outstream << "  " << l_in << "\n+ "
            << std::string(l_in.length() - s_in.length(), ' ') << s_in << "\n"
            << std::string(l_in.length() + 2, '-') << "\n"
            << std::string(l_in.length() + 2 - answer.length(), ' ')
            << "\u001b[32m" << answer << "\u001b[0m\n";
}

namespace arithmetica_factor_polynomial {
std::string factor_polynomial(std::string expr, std::vector<std::string> &steps,
                              bool show_steps);
};

std::string funcsub(std::map<std::string, algnum::algexpr> &user_defined_funcs,
                    std::string func_name, std::vector<std::string> &orig_vals,
                    std::vector<std::string> &new_vals) {
  // If the function exists in the map
  std::string s;
  if (user_defined_funcs.find(func_name) != user_defined_funcs.end()) {
    s = user_defined_funcs[func_name].to_string();
  } else {
    s = algnum::algexpr(func_name.c_str()).to_string();
  }
  size_t num_vars = orig_vals.size();
  for (size_t i = 0; i < num_vars; ++i) {
    replace_all(s, "(" + orig_vals[i] + ")", "(" + new_vals[i] + ")");
  }
  // Since my algebraic parser is .. not that great, it doesn't really support
  // evaluating normal arithmetic expressions. So we'll add a check to see if
  // our expression does not have any variables anymore. If so, we'll use the
  // much better arithmetic expression parser, otherwise, we'll settle with the
  // broken algebraic parser.
  if (is_valid_arithmetic_expression(s)) {
    return arithmetica::simplify_arithmetic_expression(s, 1, accuracy);
  } else {
    algnum::algexpr e(s.c_str());
    std::stringstream ss;
    ss << e;
    return ss.str();
  }
}

bool check_for_implicit_eval(std::string &s) {
  if (is_valid_arithmetic_expression(s)) {
    s = "eval " + s;
    return true;
  }
  return false;
}

std::vector<std::vector<arithmetica::Fraction>>
invert_matrix(std::vector<std::vector<arithmetica::Fraction>> a,
              bool &possible);

std::string
prettify_matrix(std::vector<std::vector<arithmetica::Fraction>> table) {
  if (table.empty())
    return "";

  // get the max lengths of each row
  std::vector<size_t> max_len;
  for (size_t i = 0; i < table[0].size(); i++) {
    max_len.push_back(0);
    for (size_t j = 0; j < table.size(); j++)
      max_len[i] = std::max(max_len[i], table[j][i].to_string().length());
  }

  // start making table
  std::string bar = "┌-";
  for (auto &i : max_len)
    bar += std::string(i + 2, ' ') + " ";
  bar.pop_back();
  bar.pop_back();
  bar.pop_back();
  bar += "-┐";
  std::string answer = bar;
  for (size_t i = 0; i < table.size(); i++) {
    answer += "\n|";
    for (size_t j = 0; j < table[i].size(); j++) {
      answer += " " + table[i][j].to_string();
      if (max_len[j] - table[i][j].to_string().length() > 0) {
        answer +=
            std::string(max_len[j] - table[i][j].to_string().length(), ' ');
      }
      answer += " ";
      if (j != table[i].size() - 1) {
        answer += " ";
      } else {
        answer += "|";
      }
    }
  }
  std::string bottom_bar = "└-";
  for (auto &i : max_len)
    bottom_bar += std::string(i + 2, ' ') + " ";
  bottom_bar.pop_back();
  bottom_bar.pop_back();
  bottom_bar.pop_back();
  bottom_bar += "-┘";
  answer += "\n" + bottom_bar;

  return answer;
}

std::vector<std::vector<arithmetica::Fraction>> parse_matrix(std::string str) {
  std::vector<std::vector<arithmetica::Fraction>> a;
  str = str.substr(1, str.length() - 2);
  std::string cur;
  for (int i = 0, bal = 0; i < str.length(); ++i) {
    bal += str[i] == '(';
    bal -= str[i] == ')';
    if (bal == 1 and str[i] == '(') {
      cur.clear();
      continue;
    }
    if (bal == 0 and str[i] == ')') {
      auto t = tokenize(cur, ',');
      std::vector<arithmetica::Fraction> v;
      for (auto &i : t) {
        v.emplace_back(i);
      }
      a.push_back(v);
      continue;
    }
    cur.push_back(str[i]);
  }
  return a;
}

std::string matrix_to_line(std::vector<std::vector<arithmetica::Fraction>> a) {
  std::string ans = "{";
  for (int i = 0; i < a.size(); ++i) {
    ans += "{";
    for (int j = 0; j < a[i].size(); ++j) {
      ans += a[i][j].to_string();
      if (j != a[i].size() - 1) {
        ans += ", ";
      }
    }
    ans += "}";
    if (i != a.size() - 1) {
      ans += ",";
    }
    if (i != a.size() - 1) {
      ans += " ";
    }
  }
  ans += "}";
  return ans;
}

std::vector<std::vector<arithmetica::Fraction>>
matmul(std::vector<std::vector<arithmetica::Fraction>> &a,
       std::vector<std::vector<arithmetica::Fraction>> &b);

int arithmetica_tui(int argc, char **argv, std::istream &instream_,
                    std::ostream &outstream_) {
  using namespace basic_math_operations;
  using namespace arithmetica;

  auto original_cout = outstream.rdbuf();
  auto original_cin = instream.rdbuf();

  outstream.rdbuf(outstream_.rdbuf());
  instream.rdbuf(instream_.rdbuf());

  std::string printable_version = "arithmetica ";

  std::vector<std::string> history;
  std::string prev_result;
  int history_index = -1; // Current history item

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
  bool show_steps = false;
  bool degree_mode = true;
  bool enable_fractions_eval = true;
  bool verbose_eval = false;
  bool numeric_eval = false;
  bool experimental_pretty_fractions_eval = true;

  std::map<std::string, algnum::algexpr> user_defined_funcs;

  if (argc >= 2) {
    if (std::string(argv[1]) == "--version") {
      outstream << printable_version << "\n";
      goto ret;
    }
    if (std::string(argv[1]) == "--get-tag") {
      if (!autorelease.empty() && autorelease != "0") {
        outstream << version << "-alpha-" << autorelease << "\n";
        goto ret;
      }
      outstream << version << "\n";
      goto ret;
    }

    if (std::string(argv[1]) == "--update-bleeding-edge") {
      std::string command;
#if defined(__linux__) || defined(__MACH__)
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
    outstream << "Usage: arithmetica [--version] [--get-tag] [--update] "
                 "[--update-bleeding-edge] [-i] [-o]\n";
    std::exit(0);
  }

  if (!no_introduction) {
    outstream << printable_version;

    if (!autorelease.empty() && autorelease != "0") {
      outstream << "\n"
                << "This version was automatically compiled and released by "
                   "GitHub Actions. Due to its bleeding edge nature, some "
                   "features might be unstable.\n";
    }

    outstream << "\nhttps://github.com/arithmetica-org/arithmetica-tui\n\n";

    outstream << "arithmetica supports showing working with steps (disabled "
                 "by default), toggle this by typing \"showsteps\".\n\n";

    outstream << "To get started, type help.\n";
  }

  while (true) {
    ++history_index;
    history.push_back("");
    std::string input;
    size_t input_index = 0;
    outstream << "arithmetica> ";

    char c;
#if defined(__linux__) || defined(__MACH__)
    while ((c = getch(instream)) != '\n') {
      if (c == 27) { // Escape character (arrow key)
        c = getch(instream);
        if (c == 91) { // Left square bracket
          c = getch(instream);
          if (c == 65) { // Up arrow
            if (history_index != 0) {
              history_index--;
            }
            input = history[history_index];
            input_index = input.length();
            outstream << "\33[2K\rarithmetica> " << input;
          } else if (c == 66) { // Down arrow
            if (history_index < history.size() - 1) {
              history_index++;
            }
            input = history[history_index];
            input_index = input.length();
            outstream << "\33[2K\rarithmetica> " << input;
          } else if (c == 67) { // Right arrow
            if (input_index < input.length()) {
              input_index++;
              outstream << "\rarithmetica> " << input.substr(0, input_index);
            }
          } else if (c == 68) { // Left arrow
            if (input_index != 0) {
              input_index--;
              outstream << "\rarithmetica> " << input.substr(0, input_index);
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
            outstream << "\33[2K\r\033[A";
          }
          outstream << "\33[2K\rarithmetica> " << input;
          outstream << "\rarithmetica> " << input.substr(0, input_index);
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
          outstream << "\33[2K\r\033[A";
        }
        outstream << "\33[2K\rarithmetica> " << input;
        outstream << "\rarithmetica> " << input.substr(0, input_index);
      }
    }
#endif

#ifdef _WIN32
    // Fortunately, windows supports arrow keys and history during std::getline
    // by default Which is amazing, for once windows is better than linux Thank
    // you Microsoft
    std::getline(instream, input);

    if (reprint_input) {
      outstream << "\rarithmetica> " << input << "\n";
    }
#endif

#if defined(__linux__) || defined(__MACH__)
    // Don't add consecutive duplicates or empty strings to history
    if (input.empty() ||
        (history.size() >= 2 && history[history.size() - 2] == input)) {
      history.pop_back();
    } else {
      history[history.size() - 1] = input;
    }

    history_index = history.size() - 1;
#endif

#if defined(__linux__) || defined(__MACH__)
    outstream << "\n";
#endif

    // remove front and back whitespace
    input.erase(0, input.find_first_not_of(' '));
    input.erase(input.find_last_not_of(' ') + 1);

    auto tokens = tokenize(input);

    if (input.substr(0, 4) == "help") {
      if (tokens.size() == 1) {
        outstream << "\n";
        outstream << "help - show this help message\n";
        outstream << "quickstart - show a quickstart guide\n";
        outstream
            << "showsteps - toggle showing steps (for supported functions)\n";
        outstream << "eval <expression> - evaluate an arithmetic expression\n";
        outstream << "add <number/algexpr> <number/algexpr> - add two numbers "
                     "or algebraic expressions\n";
        outstream << "mul <number/algexpr> <number/algexpr> - multiply two "
                     "numbers or algebraic expressions\n";
        outstream << "factor <polynomial> - factor a polynomial\n";
        outstream << "sin/cos/tan <angle> - trigonometric functions\n";
        outstream << "asin/acos/atan <number> - inverse trigonometric "
                     "functions\n";
        outstream << "sqrt <number> - square root\n";
        outstream << "exp <number> - compute e^<number>\n";
        outstream
            << "log <base> <number> - compute log_<base>(<number>), or ln "
               "(<number>) if <base> is not specified\n";
        outstream << "fact <number> - computes the factorial of <number>\n";
        outstream << "tocontfrac <number> - converts <number> to a continued "
                     "fraction\n";
        outstream << "gcd <number> <number> ... - computes the greatest common "
                     "divisor of the given numbers\n";
        outstream << "lcm <number> <number> ... - computes the least common "
                     "multiple of the given numbers\n";
        outstream << "funcadd <name> <algexpr> - add a function to the "
                     "function list, see funclist\n";
        outstream << "funclist - list all added functions\n";
        outstream << "subt [function_name/algexpr], var1=new1, var2=new2 - "
                     "substitute variables in functions/algebraic with "
                     "constant values\n";
        outstream << "invertmatrix [matrix elements, in {{},{}} format] - "
                     "inverts an n-by-n "
                     "matrix\n";
        outstream << "matmul [matrix1] [matrix2] - multiply two matrices\n";

        outstream
            << "\nFor help with a specific function, type help <function>\n\n";
        outstream << "Options:\n";
        outstream << "  showsteps - " << (show_steps ? "true" : "false")
                  << "\n";
        outstream << "  degreemode - " << (degree_mode ? "enabled" : "disabled")
                  << "\n";
        outstream << "  fractionseval (enable fractions during eval) - "
                  << (enable_fractions_eval ? "enabled" : "disabled") << "\n";
        outstream << "  verboseeval (include verbose explanations with steps "
                     "during eval) - "
                  << (verbose_eval ? "enabled" : "disabled") << "\n";
        outstream << "  numericeval (purely numeric evaluation during eval) - "
                  << (numeric_eval ? "enabled" : "disabled") << "\n";
        outstream
            << "  experimentalprettyfractionseval (pretty print fractions "
               "during eval) - "
            << (experimental_pretty_fractions_eval ? "enabled" : "disabled")
            << "\n";
        outstream << "  accuracy - " << accuracy
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
        outstream << "experimentalprettyfractionseval is now "
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
        outstream << "Usage: experimentalprettyfractionseval <true/false>\n";
      }
      continue;
    }

    if (input.substr(0, 3) == "gcd") {

      if (tokens.size() < 2) {
        outstream << "Usage: gcd <number> <number> ...\n";
        continue;
      }

      std::string gcd = tokens[1];
      for (int i = 2; i < tokens.size(); i++) {
        gcd = arithmetica::igcd(gcd, tokens[i]);
      }

      outstream << "==> " << gcd << "\n";
    }
    if (input.substr(0, 3) == "lcm") {

      if (tokens.size() < 2) {
        outstream << "Usage: lcm <number> <number> ...\n";
        continue;
      }

      std::string lcm = tokens[1];
      for (int i = 2; i < tokens.size(); i++) {
        lcm = arithmetica::ilcm(lcm, tokens[i]);
      }

      outstream << "==> " << lcm << "\n";
    }
    if (input.substr(0, 10) == "tocontfrac") {

      if (tokens.size() != 2) {
        outstream << "Usage: tocontfrac <number>\n";
        continue;
      }

      arithmetica::Fraction f = tokens[1];
      auto answer = arithmetica::fraction_to_continued_fraction(f.numerator,
                                                                f.denominator);
      outstream << "==> [" << answer[0] << "; ";
      for (int i = 1; i < answer.size(); i++) {
        outstream << answer[i];
        if (i != answer.size() - 1) {
          outstream << ", ";
        }
      }
      outstream << "]\n";
    }
    if (tokenize(input)[0] == "fact" || tokenize(input)[0] == "factorial") {

      if (tokens.size() != 2) {
        outstream << "Usage: fact <number>\n";
        continue;
      }

      if (tokens[1].find('.') != std::string::npos) {
        outstream << "Invalid argument: " << tokens[1] << "\n";
        continue;
      }

      unsigned long long n;
      try {
        n = std::stoull(tokens[1]);
      } catch (std::invalid_argument &e) {
        outstream << "Invalid argument: " << tokens[1] << "\n";
        continue;
      } catch (std::out_of_range &e) {
        outstream << "Number too large (for now): " << tokens[1] << "\n";
        continue;
      }

      if (n == 0) {
        outstream << "==> 1\n";
        continue;
      }

      outstream << "==> " << arithmetica::factorial(n) << "\n";
    }
    if (input.substr(0, 3) == "log") {

      if (tokens.size() != 2 && tokens.size() != 3) {
        outstream << "Usage: log <base> <number>\n";
        continue;
      }

      if (tokens.size() == 2) {
        outstream << "==> "
                  << arithmetica::natural_logarithm(tokens[1], accuracy)
                  << "\n";
      } else {
        outstream
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
        outstream << "Usage: exp <number>\n";
        continue;
      }
      outstream << "==> " << arithmetica::exponential(tokens[1], accuracy)
                << "\n";
    }
    if (input.substr(0, 4) == "sqrt") {

      if (tokens.size() != 2) {
        outstream << "Usage: sqrt <number>\n";
        continue;
      }
      std::string ans = arithmetica::square_root(tokens[1], accuracy);
      outstream << "==> " << ans << "\n";
    }
    if (tokens[0] == "invertmatrix") {
      if (tokens.size() != 2 or tokens[1][0] != '(' or
          tokens[1].back() != ')') {
        outstream
            << "Usage: invertmatrix [matrix elements, in {{},{}} format]\n";
        continue;
      }
      auto a = parse_matrix(tokens[1]);
      bool is_square_matrix = true;
      for (int i = 0; i < a.size(); ++i) {
        if (a[i].size() != a.size()) {
          is_square_matrix = false;
        }
      }
      if (!is_square_matrix) {
        std::cout << "A non-square matrix cannot be inverted.\n";
        continue;
      }
      bool possible = true;
      auto ans = invert_matrix(a, possible);
      if (!possible) {
        std::cout << "Matrix rows are linearly dependent: matrix cannot be "
                     "inverted.\n";
        continue;
      }
      std::cout << '\n' << prettify_matrix(ans) << "\n\n";
      std::cout << matrix_to_line(ans) << "\n";
    }
    if (tokens[0] == "matmul") {
      if (tokens.size() != 3) {
        std::cout << "Usage: matmul [matrix1] [matrix2]\n";
        continue;
      }
      auto a = parse_matrix(tokens[1]);
      auto b = parse_matrix(tokens[2]);
      auto ans = matmul(a, b);
      std::cout << '\n' << prettify_matrix(ans) << "\n\n";
      std::cout << matrix_to_line(ans) << "\n";
    }
    if (input == "numericeval") {
      numeric_eval = !numeric_eval;
      outstream << "numeric evaluation is now "
                << (numeric_eval ? "enabled" : "disabled") << "\n";
    }
    if (input == "verboseeval") {
      verbose_eval = !verbose_eval;
      outstream << "verbose evaluation is now "
                << (verbose_eval ? "enabled" : "disabled") << "\n";
    }
    if (input == "fractionseval") {
      enable_fractions_eval = !enable_fractions_eval;
      outstream << "fractions are now "
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
        outstream << "Example usage: " << example_usages[is_acos + 2 * is_atan]
                  << "\n";
        continue;
      }
      std::string num = tokens[1];
      outstream << " ==> " << functions[is_acos + 2 * is_atan] << "(" << num
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
      outstream << round_decimal(
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
        outstream << "Example usage: " << example_usages[is_cos + is_tan * 2]
                  << ", assuming that degree mode is enabled\n";
        continue;
      }
      std::string num = tokens[1];
      outstream << " ==> " << function_names[is_cos + is_tan * 2] << "(" << num
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

      outstream << round_decimal(answer, accuracy) << "\n";
    }
    if (input.substr(0, 8) == "accuracy") {

      if (tokens.size() == 1) {
        outstream << "accuracy is currently " << accuracy << "\n";
        continue;
      }
      std::string &num = tokens[1];
      if (num.length() > 18) {
        outstream << "Number too large\n";
        continue;
      }
      if (num[0] == '-') {
        outstream << "Number must be positive\n";
        continue;
      }
      try {
        accuracy = std::stoull(num);
      } catch (std::invalid_argument &e) {
        outstream << "Invalid number: " << num << "\n";
        continue;
      }
      outstream << "accuracy is now " << accuracy << "\n";
    }

    if (tokenize(input)[0] == "factor" || tokenize(input)[0] == "factorize") {
      if (input.length() < 8) {
        outstream << "Example usage: factor x^2+3x+2 => (x+1)(x+2)\n";
        continue;
      }

      std::string expression;
      for (size_t i = 1; i < tokens.size(); ++i) {
        expression += tokens[i];
      }
      std::vector<std::string> steps;
      std::string factored = arithmetica_factor_polynomial::factor_polynomial(
          expression, steps, show_steps);
      outstream << "\n";
      if (factored != "ERROR") {
        steps.push_back(factored);
        for (auto &i : steps) {
          replace_all(i, "+", " + ");
          replace_all(i, "-", " - ");
          outstream << "==> " << i << "\n";
        }
      }
      outstream << "\n";
    }

    if (input == "showsteps") {
      show_steps = !show_steps;
      outstream << "showsteps is now " << (show_steps ? "true" : "false")
                << "\n";
    }
    if (input == "degreemode") {
      degree_mode = !degree_mode;
      outstream << "degreemode is now "
                << (degree_mode ? "enabled" : "disabled") << "\n";
    }
    if (input.substr(0, 4) == "eval" || check_for_implicit_eval(input)) {
      tokens = tokenize(input);
      if (input.length() < 6) {
        outstream << "Example usage: eval 2+2 => 4\n";
        continue;
      }

      std::string expression = tokens[1];
      for (auto i = 2; i < tokens.size(); ++i) {
        expression += tokens[i];
      }

      if (numeric_eval) {
        std::string ans = arithmetica::simplify_arithmetic_expression(
            expression, 0, accuracy);
        outstream << " ==> " << ans << "\n";
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
        outstream << "\n";

        if (verbose_eval) {
          outstream << "Task: Simplify " << expression << "\n\n";
        } else if (print_original) {
          outstream << "==> " << expression << "\n";
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
            print_eval_expression(i, 1, 0, NULL, NULL, outstream);
          }
        }
        if (!experimental_pretty_fractions_eval) {
          outstream << s;
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
          outstream << "[";
          for (auto i = 0; i < to_print.size(); ++i) {
            to_print[i].erase(
                std::remove(to_print[i].begin(), to_print[i].end(), ' '),
                to_print[i].end());
            outstream << to_print[i];
            if (i != to_print.size() - 1) {
              outstream << ", ";
            }
          }
          outstream << "]\n";
        }
        if (s.length() > 1 && s[s.length() - 2] != '\n') {
          outstream << "\n";
        }
      }
    }
    if (input.substr(0, 3) == "add") {
      std::vector<std::string> tokens = tokenize(input);
      if (tokens.size() < 3) {
        outstream << "Example usage: add 1/2 1/2 => 1\n";
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

        outstream << "==> " << expr << "\n";
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
          outstream << "Task:    ";
          print_expression({tokens[1], tokens[2]}, {"+", "+"}, 9);

          arithmetica::Fraction f_1 = arithmetica::Fraction(tokens[1]);
          arithmetica::Fraction f_2 = arithmetica::Fraction(tokens[2]);
          int step = 0;

          if (f_1.numerator != tokens[1].substr(0, tokens[1].find('/')) ||
              f_2.numerator != tokens[2].substr(0, tokens[2].find('/'))) {
            ++step;
            outstream << "\nStep #" << step
                      << ": Simplify the input fractions\n";
            outstream << "    " << tokens[1] << " ==> "
                      << arithmetica::to_string(f_1) << "\n";
            outstream << "    " << tokens[2] << " ==> "
                      << arithmetica::to_string(f_2) << "\n";
            tokens[1] = arithmetica::to_string(f_1);
            tokens[2] = arithmetica::to_string(f_2);
          }

          if (f_1.denominator != f_2.denominator) {
            std::string lcm =
                arithmetica::ilcm(f_1.denominator, f_2.denominator);
            ++step;
            outstream
                << "\nStep #" << step
                << ": Find the lowest common multiple of the denominators.\n";
            outstream << "    ==> lcm(" << f_1.denominator << ", "
                      << f_2.denominator << ") = " << lcm << "\n";
            ++step;
            outstream << "\nStep #" << step
                      << ": Multiply each of the fractions by their lowest "
                         "common multiple over their denominator.\n";
            outstream << "    ";
            print_expression({tokens[1], tokens[2]}, {"+", "+"}, 4);
            outstream << "==> ";
            std::string lcm_den_1 = divide(lcm, f_1.denominator, 0);
            std::string lcm_den_2 = divide(lcm, f_2.denominator, 0);
            print_expression({lcm_den_1 + "/" + lcm_den_1, tokens[1],
                              lcm_den_2 + "/" + lcm_den_2, tokens[2]},
                             {"x", "+", "x"}, 4);
            outstream << "==> ";
            print_expression({multiply(f_1.numerator, lcm_den_1) + "/" +
                                  multiply(f_1.denominator, lcm_den_1),
                              multiply(f_2.numerator, lcm_den_2) + "/" +
                                  multiply(f_2.denominator, lcm_den_2)},
                             {"+", "+"}, 4);
          }

          ++step;
          outstream << "\nStep #" << step
                    << ": Since both fractions have the same denominator, we "
                       "can simply add their numerators.\n";
          outstream << "==> ";
          print_expression({arithmetica::to_string(f_1 + f_2)}, {""}, 4);

          continue;
        }

        // add_whole
        if (tokens[1].find('.') == std::string::npos &&
            tokens[2].find('.') == std::string::npos) {
          print_add_whole_steps(tokens[1], tokens[2]);
          continue;
        }

        outstream << "Sorry, this isn't supported yet!\n";
      }
    }
    if (input.substr(0, 3) == "mul") {
      std::vector<std::string> tokens = tokenize(input);
      if (tokens.size() < 3) {
        outstream << "Example usage: mul 1/2 1/2 => 1/4\n";
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
        outstream << "==> " << answer << "\n";
        continue;
      }

      if (!show_steps) {
        if (tokens[1].find('/') == std::string::npos &&
            tokens[2].find('/') == std::string::npos) {
          std::string ans =
              basic_math_operations::multiply(tokens[1], tokens[2]);
          outstream << "==> " << ans << "\n";
        }
      } else {
        if (tokens[1].find('/') != std::string::npos ||
            tokens[2].find('/') != std::string::npos) {
          outstream << "Task:    ";
          print_expression({tokens[1], tokens[2]}, {"x", "x"}, 9);

          arithmetica::Fraction f_1 = arithmetica::Fraction(tokens[1]);
          arithmetica::Fraction f_2 = arithmetica::Fraction(tokens[2]);
          int step = 0;

          if (f_1.numerator != tokens[1].substr(0, tokens[1].find('/')) ||
              f_2.numerator != tokens[2].substr(0, tokens[2].find('/'))) {
            ++step;
            outstream << "\nStep #" << step
                      << ": Simplify the input fractions\n";
            outstream << "    " << tokens[1] << " ==> "
                      << arithmetica::to_string(f_1) << "\n";
            outstream << "    " << tokens[2] << " ==> "
                      << arithmetica::to_string(f_2) << "\n";
            tokens[1] = arithmetica::to_string(f_1);
            tokens[2] = arithmetica::to_string(f_2);
          }

          ++step;
          outstream << "\nStep #" << step
                    << ": Multiply the numerators and denominators of the "
                       "fractions.\n";
          outstream << "    ";
          print_expression({tokens[1], tokens[2]}, {"x", "x"}, 4);
          outstream << "==> ";
          print_expression({multiply(f_1.numerator, f_2.numerator) + "/" +
                            multiply(f_1.denominator, f_2.denominator)},
                           {""}, 4);

          arithmetica::Fraction result = f_1 * f_2;
          if (result.numerator != multiply(f_1.numerator, f_2.numerator) ||
              result.denominator !=
                  multiply(f_1.denominator, f_2.denominator)) {
            ++step;
            outstream << "\nStep #" << step << ": Simplify the result.\n";
            outstream << "==> ";
            print_expression({arithmetica::to_string(result)}, {""}, 4);
          }

          continue;
        }

        // mul_whole
        if (tokens[1].find('.') == std::string::npos &&
            tokens[2].find('.') == std::string::npos) {
          // If the numbers are 1 digit, the result is trivial
          if (tokens[1].length() == 1 && tokens[2].length() == 1) {
            outstream << "==> " << std::stoi(tokens[1]) * std::stoi(tokens[2])
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

          outstream << std::string(m - l_in.length() + 2, ' ') << l_in << "\n";
          outstream << " x" << std::string(m - s_in.length(), ' ') << s_in
                    << "\n";
          outstream << std::string(m - l_in.length(), ' ')
                    << std::string(l_in.length() + 2, '-') << "\n";
          for (auto i = 0; i < to_add.size(); ++i) {
            std::string print =
                std::string(m - to_add[i].length() + 2, ' ') + to_add[i];
            if (i != 0)
              print[0] = '+';
            outstream << print << "\n";
          }
          outstream << "  " << std::string(result.length(), '-') << "\n";
          outstream << "  " << result << "\n";
          continue;
        }

        outstream << "Sorry, this isn't supported yet!\n";
      }
    }
    if (input.substr(0, 3) == "div") {

      if (tokens.size() < 3) {
        outstream << "Invalid input!\n";
        continue;
      }
      if (show_steps) {
        divide_with_steps(tokens[1], tokens[2], accuracy);
      } else {
        outstream << "==> " << divide(tokens[1], tokens[2], accuracy) << "\n";
      }
    }

    if (tokens[0] == "funcadd") {
      // funcadd [name] [algexpr]
      if (tokens.size() < 3) {
        std::cout << "Syntax: funcadd [name] [algexpr]\n";
        continue;
      }
      algnum::algexpr e(tokens[2].c_str());
      user_defined_funcs[tokens[1]] = e;
    }

    if (tokens[0] == "funclist") {
      for (auto &i : user_defined_funcs) {
        std::cout << i.first << ": " << i.second << "\n";
      }
    }

    if (tokens[0] == "subt") {
      // subt [algexpr/function name], var1=newval, var2=newval, ...
      bool bad = false;
      if (tokens.size() < 2) {
        bad = true;
      }
      std::vector<std::string> orig_vals, new_vals;
      input = input.substr(5);
      std::vector<std::string> tokens = tokenize(input, ',');
      for (size_t i = 1; i < tokens.size(); ++i) {
        std::vector<std::string> tk = tokenize(tokens[i], '=');
        if (tk.size() != 2) {
          bad = true;
          break;
        }
        orig_vals.push_back(remove_spaces(tk[0]));
        new_vals.push_back(remove_spaces(tk[1]));
      }
      if (bad) {
        std::cout << "Syntax: subt [algexpr/function name], var1=newval, "
                     "var2=newval, ...\n";
        continue;
      }
      std::cout << "==> "
                << funcsub(user_defined_funcs, remove_spaces(tokens[0]),
                           orig_vals, new_vals)
                << "\n";
    }
  }

ret:
  // Restore the cout and cin buffers
  std::cout.rdbuf(original_cout);
  std::cin.rdbuf(original_cin);
  return 0;
}