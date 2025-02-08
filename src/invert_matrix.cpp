#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
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

  for (int s = 0; s < a.size(); ++s) {
    int nonzero_idx = a.size();
    for (int i = s; i < a.size(); ++i) {
      if (!(a[i][s] == "0")) {
        nonzero_idx = i;
        break;
      }
    }
    if (nonzero_idx == a.size()) {
      possible = false;
      return ans;
    }
    if (nonzero_idx != s) {
      for (int i = 0; i < a.size(); ++i) {
        a[s][i] = a[s][i] + a[nonzero_idx][i];
        ans[s][i] = ans[s][i] + ans[nonzero_idx][i];
      }
    }
    for (int i = s + 1; i < a.size(); ++i) {
      auto k = a[i][s] / a[s][s];
      for (int j = 0; j < a.size(); ++j) {
        a[i][j] = a[i][j] - k * a[s][j];
        ans[i][j] = ans[i][j] - k * ans[s][j];
      }
    }
  }

  for (int s = a.size() - 1; s >= 0; --s) {
    for (int i = s + 1; i < a.size(); ++i) {
      auto k = a[s][i];
      for (int j = 0; j < a.size(); ++j) {
        a[s][j] = a[s][j] - k * a[i][j];
        ans[s][j] = ans[s][j] - k * ans[i][j];
      }
    }
    int nonzero_idx = a.size();
    for (int i = s; i < a.size(); ++i) {
      if (!(a[i][s] == "0")) {
        nonzero_idx = i;
        break;
      }
    }
    if (nonzero_idx == a.size()) {
      possible = false;
      return ans;
    }
    if (nonzero_idx != s) {
      for (int i = 0; i < a.size(); ++i) {
        a[s][i] = a[s][i] + a[nonzero_idx][i];
        ans[s][i] = ans[s][i] + ans[nonzero_idx][i];
      }
    }
    auto k = a[s][s];
    if (k == "0") {
      possible = false;
      return ans;
    }
    for (int i = 0; i < a.size(); ++i) {
      a[s][i] = a[s][i] / k;
      ans[s][i] = ans[s][i] / k;
    }
  }

  return ans;
}