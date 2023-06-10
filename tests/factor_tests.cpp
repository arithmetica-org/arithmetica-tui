#include <gtest/gtest.h>
#include <string>

std::string call_arithmetica_tui(std::string command);

TEST(FactorTests, Test1) {
  std::string result;
  result = call_arithmetica_tui("factor x^2+3x+2");
  ASSERT_EQ(result, "arithmetica> factor x^2+3x+2\\n\\n==> (x + 1)(x + 2)\\n\\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\nfactor x^2+3x+2");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> factor x^2+3x+2\\n\\n==> x^2 + 3x + 2\\n==> x^2 + 2x + x + 2\\n==> x(x + 2) + 1(x + 2)\\n==> (x + 1)(x + 2)\\n\\narithmetica> exit\\n");
  // Don't combine all like terms
  result = call_arithmetica_tui("factor x^2+2x+x+2");
  ASSERT_EQ(result, "arithmetica> factor x^2+2x+x+2\\n\\n==> (x + 1)(x + 2)\\n\\narithmetica> exit\\n");
  // And this combines even less like terms
  result = call_arithmetica_tui("factor x^2+2x+x+0.5+1.5");
  ASSERT_EQ(result, "arithmetica> factor x^2+2x+x+0.5+1.5\\n\\n==> (x + 1)(x + 2)\\n\\narithmetica> exit\\n");
}

TEST(FactorTests, Test2) {
  std::string result;
  result = call_arithmetica_tui("factor x^3+6x^2+11x+6");
  ASSERT_EQ(result, "arithmetica> factor x^3+6x^2+11x+6\\n\\n==> (x + 1)(x + 2)(x + 3)\\n\\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\nfactor x^3+6x^2+11x+6");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> factor x^3+6x^2+11x+6\\n\\n==> x^3 + 6x^2 + 11x + 6\\n==> x^3 + 3x^2 + 3x^2 + 9x + 2x + 6\\n==> x^2(x + 3) + 3x(x + 3) + 2(x + 3)\\n==> (x^2 + 3x + 2)(x + 3)\\n==> (x^2 + 2x + x + 2)(x + 3)\\n==> (x(x + 2) + 1(x + 2))(x + 3)\\n==> (x + 1)(x + 2)(x + 3)\\n\\narithmetica> exit\\n");
}

TEST(FactorTests, Test3) {
  std::string result;
  result = call_arithmetica_tui("factor x^2-4");
  ASSERT_EQ(result, "this test case fails rn");
  result = call_arithmetica_tui("showsteps\nfactor x^2-4");
  ASSERT_EQ(result, "this test case fails rn");
}