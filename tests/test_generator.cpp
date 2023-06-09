#include <iostream>

std::string call_arithmetica_tui(std::string command);

// This is just a utility script for me to generate tests for the TUI
// It is not part of any compiled binaries

int main() {
    std::string input;
    std::getline(std::cin, input);

    std::string res1 = call_arithmetica_tui(input);
    std::string res2 = call_arithmetica_tui("showsteps\n" + input);

    // Replace all '\\' with '\\\\'
    for (auto i = 0; i < res1.length(); ++i) {
        if (res1[i] == '\\') {
            res1.insert(i, "\\");
            ++i;
        }
    }
    for (auto i = 0; i < res2.length(); ++i) {
        if (res2[i] == '\\') {
            res2.insert(i, "\\");
            ++i;
        }
    }

    std::cout << "TEST(EvalTests, Test) {" << "\n";
    std::cout << "  std::string result;" << "\n";
    std::cout << "  result = call_arithmetica_tui(\"" << input << "\");" << "\n";
    std::cout << "  ASSERT_EQ(result, \"" << res1 << "\");" << "\n";
    std::cout << "  result = call_arithmetica_tui(\"showsteps\\n" << input << "\");" << "\n";
    std::cout << "  ASSERT_EQ(result, \"" << res2 << "\");" << "\n";
    std::cout << "}\n";
}