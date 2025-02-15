#pragma once

#include <algorithm>
#include <arithmetica.hpp>
#include <map>
#include <vector>

#include <helpers.hpp>

namespace algnum {

typedef arithmetica::Fraction rfraction;

std::string rfrac_to_latex(rfraction frac);

class variable;

class algexpr;

class algnum {
public:
  rfraction constant;
  std::vector<variable> variables;

  void add_variable(variable v);

  std::string latex();
  algnum(const char *s);
  algnum() {}
  bool is_constant();

  std::string to_string();

  algnum operator+(algnum a2);
  algnum operator*(algnum a2);
  friend std::ostream &operator<<(std::ostream &os, const algnum n);
  bool operator==(algnum a2) const;
};

class algexpr {
private:
  void clean_double_signs(std::string &expression);

public:
  std::vector<algnum> expr;

  algexpr(const char *s);
  algexpr() {}

  std::string latex();

  algnum element(size_t index) const;
  size_t size() const;
  void insert(algnum n);
  bool is_constant();

  algexpr combine_like_terms(algexpr e);
  std::string to_string();

  algexpr operator+(algexpr e2);
  algexpr operator*(algexpr e2);

  friend std::ostream &operator<<(std::ostream &os, const algexpr n);
  bool operator==(algexpr e2) const;
  void operator=(rfraction r);
  void operator=(std::string s);
  void operator=(const char *cstr);
};

class variable {
public:
  std::string var;
  algexpr power;
  bool constant = false; // for stuff like sqrt(2)
  bool function = false; // for sin(), log(), arctan(), etc.

  std::string functionName;
  algexpr functionValue;

  variable(std::string v, rfraction p);
  variable(std::string v, std::string p);
  variable(const char *cstr);

  variable() {}

  bool operator==(const variable &v2) const;
  bool operator<(const variable &v2) const;
};
} // namespace algnum