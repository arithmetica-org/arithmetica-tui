#include <arithmetica.h>
#include <arithmetica.hpp>
#include <basic_math_operations.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

namespace eval_with_steps {

static void str_replace_all(char **str_in, const char *from, const char *to) {
  char *str = *str_in;
  size_t from_len = strlen(from);
  if (from_len == 0)
    return;
  // algorithm here
  size_t len = strlen(str) + strlen(to) + 1;
  char *new_str = (char *)calloc(len, 1);
  size_t characters_added = 0;
  size_t str_len = strlen(str);
  for (size_t i = 0; i < str_len; i++) {
    const char *loc = strstr(str + i, from);
    bool found = loc == str + i;
    size_t _characters_added = characters_added;
    if (found)
      characters_added += strlen(to);
    else
      characters_added++;
    if (characters_added >= len) {
      size_t old_len = len;
      len *= 2;
      new_str = (char *)realloc(new_str, len);
      memset(new_str + _characters_added, 0, old_len);
    }
    if (found) {
      memcpy(new_str + _characters_added, to, strlen(to));
      i += strlen(from) - 1;
    } else
      new_str[_characters_added++] = str[i];
  }
  new_str = (char *)realloc(new_str, strlen(new_str) + 1);
  new_str[strlen(new_str)] = 0;
  free(str);
  *str_in = new_str;
  return;
}

std::string expr_without_plus_zero(std::string expr) {
  for (size_t i = 0; i < expr.length(); ++i) {
    std::string substr = expr.substr(i, expr.length());
    if (substr == "+0") {
      // Remove the +0
      expr.replace(i, 2, "");
      i -= 2;
      continue;
    }
    if (substr.substr(0, 2) == "+0" && substr.length() >= 3) {
      if (std::string(1, substr[2]).find_first_of("+-*/^") != std::string::npos) {
        // Remove the +0
        expr.replace(i, 2, "");
        i -= 2;
      }
    }
  }

  char *expr_c = (char *)malloc(expr.length() + 1);
  strcpy(expr_c, expr.c_str());

  str_replace_all(&expr_c, "[", "(");
  str_replace_all(&expr_c, "]", ")");

  std::string answer = std::string(expr_c);
  free(expr_c);

  return answer;
}

static void remove_misplaced_and_redundant_signs(char **expression_in) {
  char *expression = *expression_in;
  if (expression[0] == '+') {
    size_t n = strlen(expression);
    memmove(expression, expression + 1, strlen(expression) - 1);
    expression[n - 1] = 0;
  }
  while (strstr(expression, "--") != NULL)
    str_replace_all(&expression, "--", "+");
  while (strstr(expression, "++") != NULL)
    str_replace_all(&expression, "++", "+");
  while (strstr(expression, "-+") != NULL)
    str_replace_all(&expression, "-+", "-");
  while (strstr(expression, "+-") != NULL)
    str_replace_all(&expression, "+-", "-");
  str_replace_all(&expression, "*+", "*");
  str_replace_all(&expression, "/+", "/");
  str_replace_all(&expression, "^+", "^");
  *expression_in = expression;
  return;
}

size_t get_corresponding_closing_bracket(const char *str, size_t index) {
  char openBracket = '(', closeBracket = ')';
  if (str[index] == '[') {
    openBracket = '[';
    closeBracket = ']';
  } else if (str[index] == '{') {
    openBracket = '{';
    closeBracket = '}';
  } else if (str[index] != '(')
    return -1;
  int count = 0;
  for (size_t i = index; i < strlen(str); i++) {
    if (str[i] == openBracket)
      count++;
    if (str[i] == closeBracket)
      count--;
    if (!count)
      return i;
  }
  return -1;
}

static void replace_substring_from_position(size_t startPosition,
                                            size_t endPosition,
                                            char **string_in,
                                            const char *replacement) {
  char *string = *string_in;

  size_t replacement_len = strlen(replacement);

  char *answer = (char *)calloc(
      strlen(string) - (endPosition - startPosition + 1) + replacement_len + 1,
      1);

  for (size_t i = 0; i < startPosition; i++)
    answer[i] = string[i];
  strcpy(answer + startPosition, replacement);
  for (size_t i = endPosition + 1; i < strlen(string); i++)
    answer[startPosition + replacement_len + i - endPosition - 1] = string[i];

  free(string);
  *string_in = answer;
  return;
}

long find_operational_sign(const char *expression, char sign) {
  bool numberFound = false;
  bool noExponent = sign != '^';
  size_t find = noExponent ? 0 : strlen(expression) - 1;
  while (noExponent ? find < strlen(expression) : find + 1 > 0) {
    if (isdigit(expression[find]))
      numberFound = true;
    if (expression[find] == sign && numberFound)
      return (long)find;
    noExponent ? find++ : find--;
  }
  // Not found
  return -1;
}

size_t get_back_corresponding_bracket(const char *str, size_t index) {
  char openBracket = '(', closeBracket = ')';
  if (str[index] == ']') {
    openBracket = '[';
    closeBracket = ']';
  } else if (str[index] == '}') {
    openBracket = '{';
    closeBracket = '}';
  } else if (str[index] != ')')
    return -1;
  int count = 0;
  for (size_t i = index; i + 1 > 0; i--) {
    if (str[i] == openBracket)
      count--;
    if (str[i] == closeBracket)
      count++;
    if (!count)
      return i;
  }
  return -1;
}

static bool equal_to_any_from(const char *options, char n, size_t size) {
  for (size_t i = 0; i < size; i++)
    if (options[i] == n)
      return true;
  return false;
}

char *get_numerical_arguments(const char *expression, bool fromLeft,
                              long *signIndexIn, int outputType) {
  int bracket_count = 0;

  char sign = expression[*signIndexIn];

  long signIndex = *signIndexIn;
  char *operators = (char *)calloc(10, 1);
  size_t numberOfOperators = 3;
  strcpy(operators, "^*+");
  if (outputType == 0) {
    strcpy(operators + 3, "/");
    numberOfOperators++;
  }

  bool encounteredNumber = false;
  size_t start = 0, length = 0; // for the final answer
  if (fromLeft) {
    if (expression[signIndex] == '^') {
      strcpy(operators + numberOfOperators, "-/");
      numberOfOperators += 2;
    }
    bool encounteredMinusSign = false;
    signIndex--;

    // Check for brackets
    if (signIndex >= 0 && (expression[signIndex] == ']') ||
        (expression[signIndex] == '}') || (expression[signIndex] == ')')) {
      size_t back_corresponding_square_bracket =
          get_back_corresponding_bracket(expression, signIndex);
      start =
          back_corresponding_square_bracket + (expression[signIndex] == ']');
      length = signIndex - back_corresponding_square_bracket + 1 -
               2 * (expression[signIndex] == ']');
      signIndex = back_corresponding_square_bracket;

      if (sign == '+' || sign == '-') {
        --signIndex;
        goto eval_w_steps_left_fetch_no_brackets;
      }
    } else {
    eval_w_steps_left_fetch_no_brackets:
      // No brackets
      while (signIndex >= 0 &&
             !equal_to_any_from(operators, expression[signIndex],
                                numberOfOperators) &&
             !encounteredMinusSign) {
        if (signIndex >= 0) {
          if (expression[signIndex] == ')') {
            bracket_count--;
          }
          if (expression[signIndex] == '(') {
            bracket_count++;
          }
        }

        if (bracket_count != 0) {
          length++;
          signIndex--;
          continue;
        }

        if (expression[signIndex] == '-')
          encounteredMinusSign = true;
        length++;
        signIndex--;
      }
      signIndex++;
      start = signIndex;
    }
  } else {
    if (expression[signIndex] == '^') {
      strcpy(operators + numberOfOperators, "/");
      numberOfOperators++;
    }
    signIndex++;
    if (signIndex < strlen(expression) && (expression[signIndex] == '[') ||
        (expression[signIndex] == '{') || (expression[signIndex] == '(')) {
      size_t corresponding_closing_bracket =
          get_corresponding_closing_bracket(expression, signIndex);
      start = signIndex + (expression[signIndex] == '[');
      length = corresponding_closing_bracket - signIndex + 1 -
               2 * (expression[signIndex] == '[');
      signIndex = corresponding_closing_bracket;

      // if (sign == '+' || sign == '-') {
      //   ++signIndex;
      //   goto eval_w_steps_right_fetch_no_brackets;
      // }
    } else {
      // eval_w_steps_right_fetch_no_brackets:
      start = signIndex;
      while (signIndex < strlen(expression) &&
             (((!equal_to_any_from(operators, expression[signIndex],
                                   numberOfOperators)) &&
               !(expression[signIndex] == '-' && encounteredNumber)) ||
              bracket_count != 0)) {
        if (signIndex >= 0) {
          if (expression[signIndex] == ')') {
            bracket_count--;
          }
          if (expression[signIndex] == '(') {
            bracket_count++;
          }
        }

        if (bracket_count != 0) {
          length++;
          signIndex++;
          continue;
        }

        if (!encounteredNumber)
          encounteredNumber = isdigit(expression[signIndex]);
        length++;
        signIndex++;
      }
      signIndex--;
    }
  }

  char *answer = (char *)calloc(length + 1, 1);
  strncpy(answer, expression + start, length);
  free(operators);
  *signIndexIn = signIndex;
  return answer;
}

static void get_chain_division_location(char *expression, long *sign1In,
                                        long *sign2In) {
  long sign1 = *sign1In;
  long sign2 = *sign2In;

  long divisionSignLocation = find_operational_sign(expression, '/');
  while (divisionSignLocation >= 0) {
    long rightArgumentEnd = divisionSignLocation;
    expression[divisionSignLocation] =
        '^'; // This is just temporary, it's so we don't pull division
             // signs in the right argument.
    char *rightArgument =
        get_numerical_arguments(expression, false, &rightArgumentEnd, 1);
    expression[divisionSignLocation] = '/';
    if (rightArgumentEnd + 1 < strlen(expression) &&
        expression[rightArgumentEnd + 1] == '/') {
      free(rightArgument);
      *sign1In = divisionSignLocation;
      *sign2In = rightArgumentEnd + 1;
      return;
    }

    // Find next division sign.
    long n = find_operational_sign(expression + divisionSignLocation + 1, '/');
    if (n != -1)
      divisionSignLocation += n + 1;
    else
      divisionSignLocation = -1;
    free(rightArgument);
  }

  *sign1In = -1;
  *sign2In = -1;
  return;
}

std::string make_text_noticeable(std::string s, size_t start, size_t end) {
  return s;

  return s.substr(0, start) + "\033[38;2;105;105;105m" +
         s.substr(start, end - start + 1) + "\033[0m" +
         s.substr(end + 1, s.length());
}

char *simplify_arithmetic_expression(const char *expression_in, int outputType,
                                     size_t accuracy,
                                     std::vector<std::string> &steps,
                                     bool verbose) {
  int step = 0;
  char *expression = (char *)calloc(strlen(expression_in) + 1, 1);
  strcpy(expression, expression_in);
  str_replace_all(&expression, "[", "(");
  str_replace_all(&expression, "]", ")");
  str_replace_all(&expression, "{", "(");
  str_replace_all(&expression, "}", ")");
  str_replace_all(&expression, " ", "");

  // Multiplication can also be indicated by:
  // a*b = a(b) = (a)b = (a)(b)
  str_replace_all(&expression, ")(", ")*(");

  // a(b) = a*(b)
  char *multiplicationReplaceFrom = (char *)calloc(3, 1);
  char *multiplicationReplaceTo = (char *)calloc(4, 1);
  multiplicationReplaceFrom[1] = '(';
  multiplicationReplaceTo[1] = '*';
  multiplicationReplaceTo[2] = '(';
  for (char i = '0'; i <= '9'; i++) {
    multiplicationReplaceFrom[0] = i;
    multiplicationReplaceTo[0] = i;
    str_replace_all(&expression, multiplicationReplaceFrom,
                    multiplicationReplaceTo);
  }
  // (a)b = (a)*b
  multiplicationReplaceFrom[0] = ')';
  multiplicationReplaceTo[0] = ')';
  multiplicationReplaceTo[1] = '*';
  for (char i = '0'; i <= '9'; i++) {
    multiplicationReplaceFrom[1] = i;
    multiplicationReplaceTo[2] = i;
    str_replace_all(&expression, multiplicationReplaceFrom,
                    multiplicationReplaceTo);
  }

  free(multiplicationReplaceFrom);
  free(multiplicationReplaceTo);

  std::vector<std::string> short_steps;

  std::string expr_cpp = expression;
  if (expr_cpp != std::string(expression_in)) {
    step++;
    steps.push_back("Step #" + std::to_string(step) +
                    ": Rewrite the expression as:\n " + expr_cpp + "\n");
    short_steps.push_back("==> " + expr_cpp);
  }

  size_t n = strlen(expression);
  expression = (char *)realloc(expression, n + 3);
  strcpy(expression + n, "+0");
  expression[n + 2] = 0;

  expr_cpp = expression;

  bool outputMixedFraction = false;
  if (outputType == 2) {
    outputType = 1;
    outputMixedFraction = true;
  }

  remove_misplaced_and_redundant_signs(&expression);
  expr_cpp = expression;
  if (expr_cpp != expression) {
    expr_cpp = expression;
    step++;
    steps.push_back("Step #" + std::to_string(step) +
                    ": Remove misplaced and redundant signs to get " +
                    expr_cpp + ".");
    short_steps.push_back("==> " + expr_cpp);
  }

  const char *_loc = strchr(expression, '(');
  while (_loc != NULL) {
    size_t start = _loc - expression;
    size_t end = get_corresponding_closing_bracket(expression, start);
    char *innerExpression = (char *)calloc(end - start, 1);
    strncpy(innerExpression, expression + start + 1, end - start - 1);
    std::vector<std::string> steps_sub;
    char *simplifiedInnerExpression = simplify_arithmetic_expression(
        innerExpression, outputType, accuracy, steps_sub, verbose);
    bool print =
        std::string(simplifiedInnerExpression) != std::string(innerExpression);
    if (print) {
      if (verbose) {
        ++step;
        steps.push_back("Step #" + std::to_string(step) +
                        ": Simplify the inner expression " +
                        std::string(innerExpression));
        steps.push_back("==> " + std::string(simplifiedInnerExpression));
      } else {
        if (!short_steps.empty()) {
          short_steps.pop_back();
        }
        short_steps.push_back(
            "==> " + make_text_noticeable(
                         expr_without_plus_zero(std::string(expression)),
                         start + 1, end - 1));
        for (auto i = 0; i < steps_sub.size(); ++i) {
          short_steps.push_back("  " + steps_sub[i]);
        }
      }
    }
    // Reciprocate the fraction if not in output type zero and the previous
    // character is a division sign.
    if (start >= 1 && expression[start - 1] == '/' && outputType != 0) {
      struct fraction innerExpressionFraction =
          parse_fraction(simplifiedInnerExpression);
      // Reciprocate the fraction.
      if (innerExpressionFraction.numerator[0] == '-') {
        char *tempSwap =
            (char *)calloc(strlen(innerExpressionFraction.numerator), 1);
        strcpy(tempSwap, innerExpressionFraction.numerator + 1);
        innerExpressionFraction.numerator =
            (char *)realloc(innerExpressionFraction.numerator,
                            strlen(innerExpressionFraction.denominator) + 2);
        strcpy(innerExpressionFraction.numerator + 1,
               innerExpressionFraction.denominator);
        innerExpressionFraction.denominator = (char *)realloc(
            innerExpressionFraction.denominator, strlen(tempSwap) + 1);
        strcpy(innerExpressionFraction.denominator, tempSwap);
        free(tempSwap);
      } else {
        char *tempSwap =
            (char *)calloc(strlen(innerExpressionFraction.numerator) + 1, 1);
        strcpy(tempSwap, innerExpressionFraction.numerator);
        innerExpressionFraction.numerator =
            (char *)realloc(innerExpressionFraction.numerator,
                            strlen(innerExpressionFraction.denominator) + 1);
        strcpy(innerExpressionFraction.numerator,
               innerExpressionFraction.denominator);
        innerExpressionFraction.denominator = (char *)realloc(
            innerExpressionFraction.denominator, strlen(tempSwap) + 1);
        strcpy(innerExpressionFraction.denominator, tempSwap);
        free(tempSwap);
      }
      size_t n = strlen(innerExpressionFraction.numerator);
      simplifiedInnerExpression =
          (char *)realloc(simplifiedInnerExpression,
                          n + strlen(innerExpressionFraction.denominator) + 2);
      strcpy(simplifiedInnerExpression, innerExpressionFraction.numerator);
      if (strcmp(innerExpressionFraction.denominator,
                 "1")) { // Don't include the denominator if it's 1
        strcpy(simplifiedInnerExpression + n, "/");
        strcpy(simplifiedInnerExpression + n + 1,
               innerExpressionFraction.denominator);
      }
      // Change division to multiplication.
      expression[start - 1] = '*';

      delete_fraction(innerExpressionFraction);
    }
    bool haveToChangeBrackets =
        (end + 1 < strlen(expression) && expression[end + 1] == '^') ||
        (start >= 1 && expression[start - 1] == '^');
    if (haveToChangeBrackets) {
      // Replace the brackets with '[]' (necessary for things like (-1)^n)
      expression[start] = '[';
      expression[end] = ']';
    }
    haveToChangeBrackets
        ? replace_substring_from_position(start + 1, end - 1, &expression,
                                          simplifiedInnerExpression)
        : replace_substring_from_position(start, end, &expression,
                                          simplifiedInnerExpression);
    _loc = strchr(expression, '(');

    expr_cpp = expression;
    if (print) {
      steps.push_back("The resulting expression is:\n" +
                      expr_without_plus_zero(expr_cpp) + "\n");
      short_steps.push_back("==> " + expr_without_plus_zero(expr_cpp));
    }

    free(simplifiedInnerExpression);
    free(innerExpression);
  }

  remove_misplaced_and_redundant_signs(&expression);

  // Exponents
  // Non operational signs are things like the '-' in '-2*4'
  // There is no number behind '-', so it does not act
  // as an operator.
  // Also, in the case of exponents, we need to start looking for signs from
  // the back in order to correctly deal with cases like 3^3^3.
  long signLocation = find_operational_sign(expression, '^');
  while (signLocation >= 0) {
    long start = signLocation, end = signLocation;
    char *simplifiedExponentiation;
    char *leftArgument =
        get_numerical_arguments(expression, true, &start, outputType);
    char *rightArgument =
        get_numerical_arguments(expression, false, &end, outputType);

    ++step;
    steps.push_back("Step #" + std::to_string(step) + ": Evaluate (" +
                    std::string(leftArgument) + ")^(" +
                    std::string(rightArgument) + ")");

    if (outputType == 0)
      simplifiedExponentiation = power(leftArgument, rightArgument, accuracy);
    if (outputType == 1) {
      struct fraction fraction1 = parse_fraction(leftArgument);
      struct fraction fraction2 = parse_fraction(rightArgument);
      struct fraction answer = power_fraction(fraction1, fraction2, accuracy);
      simplifiedExponentiation = (char *)calloc(
          strlen(answer.numerator) + strlen(answer.denominator) + 4, 1);
      bool squareBrackets = start - 1 >= 0 && expression[start - 1] == '^';
      if (squareBrackets)
        simplifiedExponentiation[0] = '[';
      strncpy(simplifiedExponentiation + squareBrackets, answer.numerator,
              strlen(answer.numerator));
      if (strcmp(answer.denominator, "1")) {
        simplifiedExponentiation[strlen(answer.numerator) + squareBrackets] =
            '/';
        strncpy(simplifiedExponentiation + strlen(answer.numerator) + 1 +
                    squareBrackets,
                answer.denominator, strlen(answer.denominator));
      }
      if (squareBrackets)
        simplifiedExponentiation[strlen(simplifiedExponentiation)] = ']';
      delete_fraction(fraction1);
      delete_fraction(fraction2);
      delete_fraction(answer);
    }
    free(leftArgument);
    free(rightArgument);
    steps.push_back("==> " + std::string(simplifiedExponentiation));
    replace_substring_from_position(start, end, &expression,
                                    simplifiedExponentiation);
    expr_cpp = expression;
    steps.push_back("The resulting expression is:\n" +
                    expr_without_plus_zero(expr_cpp) + "\n");
    short_steps.push_back("==> " + expr_without_plus_zero(expr_cpp));
    free(simplifiedExponentiation);
    signLocation = find_operational_sign(expression, '^');
  }

  // This next part deals with decimal division.
  if (outputType == 0) {
    long signLocation = find_operational_sign(expression, '/');
    while (signLocation >= 0) {
      long start = signLocation, end = signLocation;
      char *numerator =
          get_numerical_arguments(expression, true, &start, outputType);
      char *denominator =
          get_numerical_arguments(expression, false, &end, outputType);
      char *quotient = (char *)calloc(
          strlen(numerator) + strlen(denominator) + accuracy + 3, 1);
      divide(numerator, denominator, quotient, accuracy);
      replace_substring_from_position(start, end, &expression, quotient);
      signLocation = find_operational_sign(expression, '/');
      free(numerator);
      free(denominator);
      free(quotient);
    }
  } else if (outputType == 1) {
    // Check for and deal with chain divisions.
    // Chain division are divisions in the form of a/b/c/d/...
    // a/b/c/d = a/(bc)/d = a/(bcd)
    long sign1 = 0, sign2 = 0;
    get_chain_division_location(expression, &sign1, &sign2);
    while (sign1 >= 0 && sign2 >= 0) {
      long numeratorStart = sign1;
      long div1End = sign1;
      long div2End = sign2;
      char *numerator = get_numerical_arguments(expression, true,
                                                &numeratorStart, outputType);
      // Again, this is just temporary (to avoid pulling division signs)
      expression[sign1] = '^';
      char *div1 =
          get_numerical_arguments(expression, false, &div1End, outputType);
      expression[sign1] = '/';
      expression[sign2] = '^'; // Avoid pulling division signs
      char *div2 =
          get_numerical_arguments(expression, false, &div2End, outputType);
      expression[sign2] = '/';
      char *denominator = (char *)calloc(strlen(div1) + strlen(div2) + 3, 1);
      multiply(div1, div2, denominator);
      char *simplifiedChainDivision =
          (char *)calloc(strlen(numerator) + strlen(denominator) + 2, 1);
      strncpy(simplifiedChainDivision, numerator, strlen(numerator));
      simplifiedChainDivision[strlen(numerator)] = '/';
      strncpy(simplifiedChainDivision + strlen(numerator) + 1, denominator,
              strlen(denominator));
      ++step;
      expr_cpp = expression;
      steps.push_back(
          "Step #" + std::to_string(step) + ": Simplify the chain division " +
          expr_cpp.substr(numeratorStart, div2End - numeratorStart + 1) +
          ":\n==> " + std::string(simplifiedChainDivision));
      replace_substring_from_position(numeratorStart, div2End, &expression,
                                      simplifiedChainDivision);
      steps.push_back("The resulting expression is:\n" +
                      expr_without_plus_zero(std::string(expression)) + "\n");
      short_steps.push_back("==> " + expr_without_plus_zero(expr_cpp));
      get_chain_division_location(expression, &sign1, &sign2);
      free(numerator);
      free(div1);
      free(div2);
      free(denominator);
      free(simplifiedChainDivision);
    }
  }

  remove_misplaced_and_redundant_signs(&expression);

  char operators[] = "*+-"; // This order is important!
  size_t numberOfOperators = 3;
  // Generic procedure following order of operations
  std::string short_step_to_add = expression;
  short_step_to_add = expr_without_plus_zero(short_step_to_add);
  for (size_t i = 0; i < numberOfOperators; i++) {
    char sign = operators[i];

    short_step_to_add = expression;
    short_step_to_add = expr_without_plus_zero(short_step_to_add);

    long signLocation = find_operational_sign(expression, sign);
    while (signLocation >= 0) {
      long start = signLocation, end = signLocation;
      char *leftArgument =
          get_numerical_arguments(expression, true, &start, outputType);
      char *rightArgument =
          get_numerical_arguments(expression, false, &end, outputType);

      bool do_step =
          !(arithmetica::Fraction(std::string(leftArgument)) == "0") &&
          !(arithmetica::Fraction(std::string(rightArgument)) == "0");

      if (!short_step_to_add.empty()) {
        if (do_step) {
          short_steps.push_back(
              "==> " + make_text_noticeable(short_step_to_add, start, end));
        } else {
          short_steps.push_back("==> " + short_step_to_add);
        }
        short_step_to_add.clear();
      }

      expr_cpp = expression;
      if (do_step) {
        ++step;
        steps.push_back("Step #" + std::to_string(step) + ": Evaluate " +
                        std::string(leftArgument) + std::string(1, sign) +
                        std::string(rightArgument));
      }
      char *operationResult;
      if (outputType == 0) {
        operationResult =
            (char *)calloc(strlen(leftArgument) + strlen(rightArgument) + 3, 1);
        switch (sign) {
        case '*':
          multiply(leftArgument, rightArgument, operationResult);
          break;
        case '+':
          add(leftArgument, rightArgument, operationResult);
          break;
        case '-':
          subtract(leftArgument, rightArgument, operationResult);
          break;
        default:
          break;
        }
      }
      if (outputType == 1) {
        struct fraction fraction1 = parse_fraction(leftArgument);
        struct fraction fraction2 = parse_fraction(rightArgument);
        struct fraction answer;
        switch (sign) {
        case '*':
          answer = multiply_fraction(fraction1, fraction2);
          break;
        case '+':
          answer = add_fraction(fraction1, fraction2);
          break;
        case '-':
          answer = subtract_fraction(fraction1, fraction2);
          break;
        default:
          answer = create_fraction("0", "1");
          break;
        }

        operationResult = (char *)calloc(
            strlen(answer.numerator) + strlen(answer.denominator) + 2, 1);
        strncpy(operationResult, answer.numerator, strlen(answer.numerator));
        if (strcmp(answer.denominator, "1")) {
          operationResult[strlen(answer.numerator)] = '/';
          strncpy(operationResult + strlen(answer.numerator) + 1,
                  answer.denominator, strlen(answer.denominator));
        }

        delete_fraction(fraction1);
        delete_fraction(fraction2);
        delete_fraction(answer);
      }

      if (do_step) {
        steps.push_back("==> " + std::string(operationResult));
      }

      // If the left argument is negative and the result is not then the
      // negative sign is lost. This is obviously unacceptable: 1/4-1/3+1/2
      // --> -1/3+1/2 = 1/6 so this becomes 1/41/6?? Instead, apped a '+'
      // if this is the case.
      if (leftArgument[0] == '-' && operationResult[0] != '-') {
        size_t n = strlen(operationResult);
        operationResult = (char *)realloc(operationResult, n + 2);
        memmove(operationResult + 1, operationResult, n + 1);
        operationResult[0] = '+';
      }

      replace_substring_from_position(start, end, &expression, operationResult);
      remove_misplaced_and_redundant_signs(&expression);

      bool simplify_fraction_step =
          expr_without_plus_zero(expr_cpp) !=
          expr_without_plus_zero(std::string(expression));

      expr_cpp = expression;
      if (do_step) {
        steps.push_back("The resulting expression is:\n" +
                        expr_without_plus_zero(expr_cpp) + "\n");
        short_step_to_add = expr_without_plus_zero(expr_cpp);
      } else if (simplify_fraction_step) {
        ++step;
        steps.push_back("Step #" + std::to_string(step) +
                        ": Simplify fractions");
        steps.push_back("The resulting expression is:\n" +
                        expr_without_plus_zero(expr_cpp) + "\n");
        short_step_to_add = expr_without_plus_zero(expr_cpp);
      }
      signLocation = find_operational_sign(expression, sign);

      free(leftArgument);
      free(rightArgument);
      free(operationResult);
    }

    remove_misplaced_and_redundant_signs(&expression);
  }

  if (!short_step_to_add.empty()) {
    short_steps.push_back("==> " + short_step_to_add);
  }

  if (steps.empty()) {
    steps.push_back("==> " + std::string(expression));
    short_steps.push_back(steps.back());
  }

  if (!verbose) {
    steps = short_steps;
  }

  // If the denominator is 1 then return early.
  struct fraction frac = parse_fraction(expression);
  if (!strcmp(frac.denominator, "1")) {
    delete_fraction(frac);
    const char *_loc = strchr(expression, '/');
    if (_loc != NULL)
      expression[_loc - expression] = 0;
    return expression;
  }

  if (!outputMixedFraction) {
    delete_fraction(frac);
    return expression;
  }

  char sign = frac.numerator[0] == '-' ? '-' : '+';
  // Ignore sign.
  if (sign == '-') {
    size_t length = strlen(frac.numerator);
    memmove(frac.numerator, frac.numerator + 1, length - 1);
    frac.numerator[length - 1] = 0;
  }

  char *denominatorBiggerCheck =
      (char *)calloc(strlen(frac.numerator) + strlen(frac.denominator) + 3, 1);
  subtract(frac.numerator, frac.denominator, denominatorBiggerCheck);
  if (denominatorBiggerCheck[0] == '-') {
    free(denominatorBiggerCheck);
    delete_fraction(frac);
    return expression;
  }

  char *quotient =
      (char *)calloc(strlen(frac.numerator) + strlen(frac.denominator) + 3, 1);
  char *remainder =
      (char *)calloc(strlen(frac.numerator) + strlen(frac.denominator) + 3, 1);
  divide_whole_with_remainder(frac.numerator, frac.denominator, quotient,
                              remainder);

  size_t expression_buf_len =
      strlen(quotient) + strlen(remainder) + strlen(frac.denominator) + 6;

  expression = (char *)realloc(expression, expression_buf_len);
  size_t charactersWritten = 0;
  if (sign == '-')
    expression[charactersWritten++] = '-';
  strncpy(expression + charactersWritten, quotient, strlen(quotient));
  charactersWritten += strlen(quotient);
  if (strcmp(remainder, "0")) {
    // If the remainder is non-zero.
    expression[charactersWritten++] = ' ';
    expression[charactersWritten++] = sign;
    expression[charactersWritten++] = ' ';
    strncpy(expression + charactersWritten, remainder, strlen(remainder));
    charactersWritten += strlen(remainder);
    expression[charactersWritten++] = '/';
    strncpy(expression + charactersWritten, frac.denominator,
            strlen(frac.denominator));
    expression[charactersWritten + strlen(frac.denominator)] =
        0; // Final null terminator.
  }

  free(quotient);
  free(remainder);
  free(denominatorBiggerCheck);
  delete_fraction(frac);

  return expression;
}

}; // namespace eval_with_steps