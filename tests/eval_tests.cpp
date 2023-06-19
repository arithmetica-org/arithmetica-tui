#include <gtest/gtest.h>
#include <string>

std::string call_arithmetica_tui(std::string command);

TEST(EvalTests, Test1) {
  std::string result;
  result = call_arithmetica_tui("eval 1/2+1/3+1/6");
  ASSERT_EQ(result, "arithmetica> eval 1/2+1/3+1/6\\n   \\n1\\n   \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 1/2+1/3+1/6");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 1/2+1/3+1/6\\n\\n    1   1   1  \\n==> - + - + -\\n    2   3   6  \\n    5   1  \\n==> - + -\\n    6   6  \\n==> 1\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test2) {
  std::string result;
  result = call_arithmetica_tui("eval 120-256+100");
  ASSERT_EQ(result, "arithmetica> eval 120-256+100\\n     \\n-36\\n     \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 120-256+100");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 120-256+100\\n\\n==> 120 - 256 + 100\\n==> 120 - 156\\n==>  - 36\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test3) {
  std::string result;
  result = call_arithmetica_tui("eval 10^2/2");
  ASSERT_EQ(result, "arithmetica> eval 10^2/2\\n    \\n50\\n    \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 10^2/2");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 10^2/2\\n\\n==> 10^2/2\\n    100  \\n==> ---\\n     2   \\n==> 50\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test4) {
  std::string result;
  result = call_arithmetica_tui("eval 1/(2+1/(3+1/4))");
  ASSERT_EQ(result, "arithmetica> eval 1/(2+1/(3+1/4))\\n               13  \\n0.4333333333 = --\\n               30  \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 1/(2+1/(3+1/4))");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 1/(2+1/(3+1/4))\\n\\n         1       \\n==> -----------\\n    2+1/(3+1/4)  \\n            1    \\n  ==> 2 + -----\\n          3+1/4  \\n            1  \\n    ==> 3 + -\\n            4  \\n        13  \\n    ==> --\\n        4   \\n              4   \\n  ==> 2 + 1 \xC3\x97 --\\n              13  \\n          4   \\n  ==> 2 + --\\n          13  \\n      30  \\n  ==> --\\n      13  \\n        13  \\n==> 1 \xC3\x97 --\\n        30  \\n    13  \\n==> --\\n    30  \\n[0.4333333333, 13/30]\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test5) {
  std::string result;
  result = call_arithmetica_tui("eval (12^2+5^2)^(1/2)");
  ASSERT_EQ(result, "arithmetica> eval (12^2+5^2)^(1/2)\\n    \\n13\\n    \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval (12^2+5^2)^(1/2)");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval (12^2+5^2)^(1/2)\\n\\n                  1   \\n==> (12^2 + 5^2)^(-)\\n                  2   \\n  ==> 12^2 + 25\\n  ==> 144 + 25\\n  ==> 169\\n           1   \\n==> (169)^(-)\\n           2   \\n==> 13\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test6) {
  std::string result;
  result = call_arithmetica_tui("eval (1+3)^(1/2)+2");
  ASSERT_EQ(result, "arithmetica> eval (1+3)^(1/2)+2\\n   \\n4\\n   \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval (1+3)^(1/2)+2");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval (1+3)^(1/2)+2\\n\\n             1       \\n==> (1 + 3)^(-) + 2\\n             2       \\n  ==> 1 + 3\\n  ==> 4\\n         1       \\n==> (4)^(-) + 2\\n         2       \\n==> 2 + 2\\n==> 4\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test7) {
  std::string result;
  result = call_arithmetica_tui("eval 1/(2+3)+1 ");
  ASSERT_EQ(result, "arithmetica> eval 1/(2+3)+1 \\n      6     1  \\n1.2 = - = 1 -\\n      5     5  \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 1/(2+3)+1 ");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 1/(2+3)+1 \\n\\n     1       \\n==> --- + 1\\n    2+3      \\n  ==> 2 + 3\\n  ==> 5\\n        1      \\n==> 1 × - + 1\\n        5      \\n    1      \\n==> - + 1\\n    5      \\n    6  \\n==> -\\n    5  \\n[1.2, 6/5, 1+1/5]\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test8) {
  std::string result;
  result = call_arithmetica_tui("eval 1/(2+1) + 1/(2-1)");
  ASSERT_EQ(result, "arithmetica> eval 1/(2+1) + 1/(2-1)\\n               4     1  \\n1.3333333333 = - = 1 -\\n               3     3  \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 1/(2+1) + 1/(2-1)");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 1/(2+1) + 1/(2-1)\\n\\n     1     1   \\n==> --- + ---\\n    2+1   2-1  \\n  ==> 2 + 1\\n  ==> 3\\n        1    1   \\n==> 1 × - + ---\\n        3   2-1  \\n  ==> 2 - 1\\n  ==> 1\\n        1          \\n==> 1 × - + 1 × 1\\n        3          \\n    1          \\n==> - + 1 × 1\\n    3          \\n    1      \\n==> - + 1\\n    3      \\n    4  \\n==> -\\n    3  \\n[1.3333333333, 4/3, 1+1/3]\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test9) {
  std::string result;
  result = call_arithmetica_tui("eval (10-15)^2");
  ASSERT_EQ(result, "arithmetica> eval (10-15)^2\\n    \\n25\\n    \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval (10-15)^2");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval (10-15)^2\\n\\n==> (10 - 15)^2\\n  ==> 10 - 15\\n  ==>  - 5\\n==> ( - 5)^2\\n==> 25\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test10) {
  std::string result;
  result = call_arithmetica_tui("eval 4/(2*3)*1");
  ASSERT_EQ(result, "arithmetica> eval 4/(2*3)*1\\n               2  \\n0.6666666666 = -\\n               3  \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 4/(2*3)*1");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 4/(2*3)*1\\n\\n     4       \\n==> --- × 1\\n    2×3      \\n  ==> 2 × 3\\n  ==> 6\\n        1      \\n==> 4 × - × 1\\n        6      \\n    2      \\n==> - × 1\\n    3      \\n    2  \\n==> -\\n    3  \\n[0.6666666666, 2/3]\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test11) {
  std::string result;
  result = call_arithmetica_tui("eval 0.1+0.1");
  ASSERT_EQ(result, "arithmetica> eval 0.1+0.1\\n      1  \\n0.2 = -\\n      5  \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 0.1+0.1");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 0.1+0.1\\n\\n==> 0.1 + 0.1\\n    1  \\n==> -\\n    5  \\n[0.2, 1/5]\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test12) {
  std::string result;
  result = call_arithmetica_tui("eval 7725/75-(24*300)/75");
  ASSERT_EQ(result, "arithmetica> eval 7725/75-(24*300)/75\\n   \\n7\\n   \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 7725/75-(24*300)/75");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 7725/75-(24*300)/75\\n\\n    7725   24\xC3\x97" "300  \\n==> ---- - ------\\n     75      75    \\n  ==> 24 \xC3\x97 300\\n  ==> 7200\\n    7725   7200  \\n==> ---- - ----\\n     75     75   \\n    7725       \\n==> ---- - 96\\n     75        \\n==> 7\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test13) {
  std::string result;
  result = call_arithmetica_tui("eval 1-(1*2)/2");
  ASSERT_EQ(result, "arithmetica> eval 1-(1*2)/2\\n   \\n0\\n   \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval 1-(1*2)/2");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval 1-(1*2)/2\\n\\n        1\xC3\x97" "2  \\n==> 1 - ---\\n         2   \\n  ==> 1 \xC3\x97 2\\n  ==> 2\\n        2  \\n==> 1 - -\\n        2  \\n==> 1 - 1\\n==> 0\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test14) {
  std::string result;
  result = call_arithmetica_tui("eval (4*1)^(1/2)-1");
  ASSERT_EQ(result, "arithmetica> eval (4*1)^(1/2)-1\\n   \\n1\\n   \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval (4*1)^(1/2)-1");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval (4*1)^(1/2)-1\\n\\n             1       \\n==> (4 \xC3\x97 1)^(-) - 1\\n             2       \\n  ==> 4 \xC3\x97 1\\n  ==> 4\\n         1       \\n==> (4)^(-) - 1\\n         2       \\n==> 2 - 1\\n==> 1\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test15) {
  std::string result;
  result = call_arithmetica_tui("eval (2*8+9)^(1/2)-(8-4)^(1/2)");
  ASSERT_EQ(result, "arithmetica> eval (2*8+9)^(1/2)-(8-4)^(1/2)\\n   \\n3\\n   \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval (2*8+9)^(1/2)-(8-4)^(1/2)");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval (2*8+9)^(1/2)-(8-4)^(1/2)\\n\\n                 1             1   \\n==> (2 \xC3\x97 8 + 9)^(-) - (8 - 4)^(-)\\n                 2             2   \\n  ==> 2 \xC3\x97 8 + 9\\n  ==> 16 + 9\\n  ==> 25\\n          1             1   \\n==> (25)^(-) - (8 - 4)^(-)\\n          2             2   \\n  ==> 8 - 4\\n  ==> 4\\n          1               \\n==> (25)^(-) - (4)^(1/2)\\n          2               \\n          1       \\n==> (25)^(-) - 2\\n          2       \\n==> 5 - 2\\n==> 3\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test16) {
  std::string result;
  result = call_arithmetica_tui("eval -(1)^(1/1)");
  ASSERT_EQ(result, "arithmetica> eval -(1)^(1/1)\\n    \\n-1\\n    \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval -(1)^(1/1)");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval -(1)^(1/1)\\n\\n==>  - (1)^(1/1)\\n      1  \\n  ==> -\\n      1  \\n  ==> 1\\n==>  - (1)^(1)\\n==>  - 1\\n\\narithmetica> exit\\n");
}

TEST(EvalTests, Test17) {
  std::string result;
  result = call_arithmetica_tui("eval -1^(1/1)");
  ASSERT_EQ(result, "arithmetica> eval -1^(1/1)\\n    \\n-1\\n    \\narithmetica> exit\\n");
  result = call_arithmetica_tui("showsteps\neval -1^(1/1)");
  ASSERT_EQ(result, "arithmetica> showsteps\\nshowsteps is now true\\narithmetica> eval -1^(1/1)\\n\\n          1   \\n==>  - 1^(-)\\n          1   \\n      1  \\n  ==> -\\n      1  \\n  ==> 1\\n==>  - 1^(1)\\n==>  - 1\\n\\narithmetica> exit\\n"); 
}