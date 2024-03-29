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
  ASSERT_EQ(result, "arithmetica> factor x^2-4\\n\\n==> (x - 2)(x + 2)\\n\\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\nfactor x^2-4");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> factor x^2-4\\n\\n==> x^2 - 4\\n==> x^2 + 2x - 2x - 4\\n==> x(x + 2) - 2(x + 2)\\n==> (x - 2)(x + 2)\\n\\narithmetica> exit\\n");
}

TEST(FactorTests, Test4) {
  std::string result;
  result = call_arithmetica_tui("factor x^4-25x^2+144");
  ASSERT_EQ(result, "arithmetica> factor x^4-25x^2+144\\n\\n==> (x - 4)(x + 4)(x - 3)(x + 3)\\n\\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\nfactor x^4-25x^2+144");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> factor x^4-25x^2+144\\n\\n==> x^4 - 25x^2 + 144\\n==> x^4 + 3x^3 - 3x^3 - 9x^2 - 16x^2 - 48x + 48x + 144\\n==> x^3(x + 3) - 3x^2(x + 3) - 16x(x + 3) + 48(x + 3)\\n==> (x^3 - 3x^2 - 16x + 48)(x + 3)\\n==> (x^2(x - 3) - 16(x - 3))(x + 3)\\n==> (x^2 - 16)(x - 3)(x + 3)\\n==> (x^2 + 4x - 4x - 16)(x - 3)(x + 3)\\n==> (x(x + 4) - 4(x + 4))(x - 3)(x + 3)\\n==> (x - 4)(x + 4)(x - 3)(x + 3)\\n\\narithmetica> exit\\n");
}