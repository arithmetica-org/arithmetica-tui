#include "helpers.hpp"

void print_expression(std::vector<std::string> terms,
                      std::vector<std::string> signs, int padding,
                      std::ofstream *file, std::vector<std::string> *out,
                      size_t padding_exclude) {
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
