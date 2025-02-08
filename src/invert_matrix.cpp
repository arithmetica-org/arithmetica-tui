#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::vector<arithmetica::Fraction>>
invert_matrix(std::vector<std::vector<arithmetica::Fraction>> a,
              bool &possible) {
  std::vector ans(a.size(), std::vector<arithmetica::Fraction>(a.size(), "0"));
  for (int i = 0; i < a.size(); ++i) {
    ans[i][i] = "1";
  }

  std::function<void(int)> invert;
  invert = [&](int s) {
    if (a[s][s] == "0") {
      possible = false;
      a[s][s] = "1";
    }
    for (int i = s + 1; i < a.size(); ++i) {
      auto k = a[i][s] / a[s][s];
      for (int j = 0; j < a.size(); ++j) {
        a[i][j] = a[i][j] - k * a[s][j];
        ans[i][j] = ans[i][j] - k * ans[s][j];
      }
    }
    if (s != a.size() - 1) {
      invert(s + 1);
    }
    for (int i = s + 1; i < a.size(); ++i) {
      auto k = a[s][i];
      for (int j = 0; j < a.size(); ++j) {
        a[s][j] = a[s][j] - k * a[i][j];
        ans[s][j] = ans[s][j] - k * ans[i][j];
      }
    }
    auto k = a[s][s];
    if (k == "0") {
      possible = false;
      k = "1";
    }
    for (int i = 0; i < a.size(); ++i) {
      a[s][i] = a[s][i] / k;
      ans[s][i] = ans[s][i] / k;
    }
  };

  invert(0);
  return ans;
}
