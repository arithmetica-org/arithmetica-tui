#include "helpers.hpp"

std::vector<std::string> tokenize(std::string s, char ch) {
  // Tokenize on the character ch, essentially splitting the string into its
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

    if (s[i] == ch) {
      tokens.push_back(token);
      token.clear();
      continue;
    }
    token += s[i];
  }
  tokens.push_back(token);
  return tokens;
}