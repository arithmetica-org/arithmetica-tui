// Copyright 2022 avighnac

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include <basic_math_operations.hpp>

using namespace basic_math_operations;

namespace algebraic_num {
struct variable {
  std::string var;
  int power = 1;
};

class algebraic_number {
private:
  size_t find_first_of(size_t, size_t, std::string, std::string);

  //  Note that the number will be constantPart * variablePart
  //  and not '+' as any + or - signs will split the term into
  //  two.

public:
  std::string constantPart;
  std::map<char, float> variablePart;

  explicit algebraic_number(std::string);
  void print_number();
  std::string get_formatted_number();
  std::string char_to_str(char);
  ~algebraic_number();
};

algebraic_number::algebraic_number(std::string term) {
  //  10^2a^2b^2
  //  constant: 10^2
  //  variable: {{a, power: 2}, {b, power: 2}}

  std::string constantString = "1";
  bool isConstantStringModified = false;

  for (auto &i : term) {
    if (!('a' <= i && i <= 'z')) {
      if (!isConstantStringModified) {
        isConstantStringModified = true;
        constantString.clear();
      }
      constantString.push_back(i);
    } else { // If current character is a letter
      // This also means all of the constant part has now been taken care of.
      break;
    }
  }

  if (constantString !=
      "1") // This gets rid of the constant part from the string.
    term = term.substr(constantString.length(), term.length());

  if (constantString.find('^') == std::string::npos) {
    constantPart = constantString; // Assign value to variable.
    constantString.clear();
  } else {
    std::string constantNonPowerPart;
    int count = 0;
    for (auto &i : constantString) {
      if (i == '^')
        break;
      constantNonPowerPart.push_back(i);
      count++;
    }
    bool negative = false;
    if (constantNonPowerPart[0] == '-') {
      constantNonPowerPart.erase(0, 1);
      negative = true;
    }
    std::string constantPowerPart =
        constantString.substr(count + 1, constantString.length() - count + 1);
    std::string finalAnswer = "1";
    for (auto i = 0; i < abs(std::stoll(constantPowerPart)); i++)
      finalAnswer = multiply(finalAnswer, constantNonPowerPart);
    if (std::stoll(constantPowerPart) >= 0)
      constantPart = finalAnswer;
    else
      constantPart = divide("1", finalAnswer, finalAnswer.length() * 3);
    if (negative)
      constantPart.insert(0, 1, '-');
  }

  if (constantPart == "+0" || constantPart == "-0" || constantPart == "0")
    variablePart = {};
  else {
    char currentVar;

    for (auto i = 0; i < static_cast<int>(term.length()); i++) {
      auto &cv = term[i];
      bool justInserted = false;
      if ('a' <= cv && cv <= 'z') {
        if (variablePart.find(cv) == variablePart.end()) {
          if (term[i + 1] == '^') {
            variablePart.insert({cv, 0});
          } else {
            justInserted = true;
            variablePart.insert({cv, 1});
          }
          currentVar = cv;
        }
        if (term[i + 1] != '^' && !justInserted) {
          variablePart.find(cv)->second++;
          justInserted = false;
        }
        if (i + 2 <= term.length() - 1) {
          if (term[i + 1] == '^') {
            variablePart.find(cv)->second += std::stoi(
                term.substr(i + 2, find_first_of(i + 2, term.length() - 1,
                                                 "letter", term)));
          }
        }
      }
    }
  }
}

algebraic_number::~algebraic_number() {}

void algebraic_number::print_number() {
  std::cout << "Constant Part: " << constantPart << "\nVariables: {";

  std::string toPrint;
  for (auto &i : variablePart) {
    toPrint += "{" + char_to_str(i.first) +
               ", power: " + std::to_string(i.second) + "}, ";
  }
  toPrint = toPrint.substr(0, toPrint.length() - 2);

  std::cout << toPrint << "}\n";
}

std::string algebraic_number::get_formatted_number() {
  std::string answer;
  answer += "{" + constantPart + ", {";
  for (auto &i : variablePart) {
    answer += "{" + char_to_str(i.first) +
              ", power: " + std::to_string(i.second) + "}, ";
  }
  if (!variablePart.empty())
    answer = answer.substr(0, answer.length() - 2);

  return answer + "}}";
}

size_t algebraic_number::find_first_of(size_t startPos, size_t endPos,
                                      std::string argument, std::string term) {

  if (argument == "letter") {
    for (auto i = startPos; i <= endPos; i++) {
      if ('a' <= term[i] && term[i] <= 'z')
        return i;
    }

    return -1;
  }

  return -1;
}

std::string algebraic_number::char_to_str(char charac) {
  std::string answer;
  answer.push_back(charac);
  return answer;
}

void erase_algebraic_number(
    std::vector<algebraic_num::algebraic_number> &algebraicTerms, size_t index) {
  std::vector<algebraic_num::algebraic_number> answer;
  for (auto i = 0; i < algebraicTerms.size(); i++)
    if (i != index)
      answer.push_back(algebraicTerms[i]);
  algebraicTerms = answer;
}

std::string convert_to_readable(
    const std::vector<algebraic_num::algebraic_number> &algebraicTerms) {
  std::string answer;
  int counter = 0;
  for (auto &i : algebraicTerms) {
    if (i.constantPart[0] != '-')
      answer += "+ ";
    else
      (counter == 0) ? answer += "-" : answer += "- ";
    if ((i.constantPart != "1" && i.constantPart != "-1") ||
        i.variablePart.empty()) {
      if (i.constantPart[0] == '-')
        answer += i.constantPart.substr(1, i.constantPart.length() - 1);
      else
        answer += i.constantPart;
    }

    for (auto &j : i.variablePart) {
      answer.push_back(j.first);
      if (j.second != 1)
        answer += "^" + std::to_string(j.second);
    }
    answer += " ";
    counter++;
  }
  if (!answer.empty())
    if (answer[0] == '+')
      return answer.substr(2, answer.length() - 2);
  return answer;
}

std::vector<algebraic_number> get_terms(std::string expression,
                                       bool debugPrint = false) {
  expression.erase(std::remove(expression.begin(), expression.end(), ' '),
                   expression.end()); // Remove spaces
  if (!(expression.length() == 0)) {
    if (expression[0] != '+' && expression[0] != '-')
      expression = "+" + expression;
  } else
    return {};
  std::vector<std::string> terms;
  std::string currentTerm;
  for (auto i = 1; i < expression.length(); i++) {
    if (expression[i] == '+' ||
        expression[i] == '-' && expression[i - 1] != '^') {
      terms.push_back(currentTerm);
      currentTerm.clear();
      currentTerm.push_back(expression[i]);
    } else
      currentTerm.push_back(expression[i]);
  }
  terms.push_back(currentTerm);
  if (!terms.empty())
    terms[0] = expression.substr(0, 1) + terms[0];

  std::map<char, int> numbers;
  for (auto i = '0'; i <= '9'; i++)
    numbers.insert({i, i - 48});

  std::vector<algebraic_number> nums;
  int counter = 0;
  for (auto &i : terms) {
    counter++;
    if (numbers.find(i[1]) ==
        numbers.end()) // If the second character ([0] will be the sign)
                       // of the first term is not a number
      i = i.substr(0, 1) + "1" +
          i.substr(1,
                   i.length() -
                       1); // This converts something like -x^2 to -1x^2
    if (i[0] == '+')
      i = i.substr(1, i.length() - 1);

    algebraic_num::algebraic_number toPB(i);
    if (debugPrint) {
      std::cout << counter << ".";
      for (auto i = 0; i < 5 - std::to_string(counter).length(); i++)
        std::cout << " ";
      std::cout << toPB.get_formatted_number() << '\n';
    }
    nums.push_back(toPB);
  }
  return nums;
}

algebraic_number asquare(algebraic_number number) {
  algebraic_number answer(multiply(number.constantPart, number.constantPart));
  for (auto &i : number.variablePart)
    answer.variablePart.insert({i.first, i.second * 2});
  return answer;
}

static std::vector<int> turn_to_digits(const std::string &str_n) {
  std::vector<int> answer;
  for (auto i : str_n)
    answer.push_back(i - 48);
  return answer;
}

bool smaller_than(algebraic_number a1, algebraic_number b1) {
  if (multiply(a1.constantPart, "1") == multiply(b1.constantPart, "1"))
    return false;

  std::string a = a1.constantPart;
  std::string b = b1.constantPart;

  std::reverse(a.begin(), a.end());
  std::reverse(b.begin(), b.end());

  std::vector<int> a_vec = turn_to_digits(a);
  std::vector<int> b_vec = turn_to_digits(b);
  if (a.length() > b.length())
    for (auto i = 0; i < a.length() - b.length(); i++)
      b_vec.push_back(0);
  if (a.length() < b.length())
    for (auto i = 0; i < b.length() - a.length(); i++)
      a_vec.push_back(0);
  std::reverse(a_vec.begin(), a_vec.end());
  std::reverse(b_vec.begin(), b_vec.end());

  for (int i = 0; i < a_vec.size(); i++) {
    if (a_vec[i] > b_vec[i])
      return false;
    if (a_vec[i] < b_vec[i])
      return true;
  }

  return true;
}

bool greater_than(algebraic_number a1, algebraic_number b1) {
  return (multiply(a1.constantPart, "1") != multiply(b1.constantPart, "1")) &&
         !smaller_than(a1, b1);
}
} // namespace algebraic_num
