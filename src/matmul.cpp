#include <arithmetica.hpp>
#include <vector>

std::vector<std::vector<arithmetica::Fraction>>
matmul(std::vector<std::vector<arithmetica::Fraction>> &a,
       std::vector<std::vector<arithmetica::Fraction>> &b) {
  // a [p x q]
  // b [q x r]
  std::vector ans(a.size(),
                  std::vector<arithmetica::Fraction>(b[0].size(), "0"));
  for (int i = 0; i < a.size(); ++i) {
    for (int j = 0; j < b[0].size(); ++j) {
      for (int k = 0; k < a[0].size(); ++k) {
        ans[i][j] = ans[i][j] + a[i][k] * b[k][j];
      }
    }
  }
  return ans;
}