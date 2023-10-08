#include <basic_math_operations.hpp>
#include <iostream>
#include <string>
#include <vector>

void divide_with_steps(const std::string &dividend_,
                       const std::string &divisor_, std::size_t accuracy) {
  using namespace basic_math_operations;

  std::string dividend = add(dividend_, "0"), divisor = add(divisor_, "0");

  // Remove the decimal point by multiplying and dividing by a power of 10.
  if (dividend.find('.') != std::string::npos ||
      divisor.find('.') != std::string::npos) {
    std::size_t max_decimals =
        std::max(dividend.length() - dividend.find('.') - 1,
                 divisor.length() - divisor.find('.') - 1);
    std::string ten_power = "1" + std::string(max_decimals, '0');
    dividend = multiply(dividend, ten_power);
    divisor = multiply(divisor, ten_power);
  }

  std::string quotient;

  std::vector<std::string> multiplication_table(10);
  for (char i = '0'; i <= '9'; ++i) {
    multiplication_table[i - '0'] = multiply(divisor, std::string(1, i));
  }

  std::string current_digits = "0";
  std::vector<std::string> things_to_subtract, carry_down_res;
  bool decimal_added = false;

  for (std::size_t i = 0; i < dividend.length() + accuracy; ++i) {
    char c;
    if (i < dividend.length()) {
      c = dividend[i];
    } else {
      if (!decimal_added) {
        decimal_added = true;
        quotient.push_back('.');
      }
      c = '0';
    }
    current_digits += std::string(1, c);

    int j = 9;
    for (; j > -1; --j) {
      std::string sub = subtract(multiplication_table[j], current_digits);
      if (sub[0] == '-' || sub == "0" || sub == "-0") {
        break;
      }
    }

    things_to_subtract.push_back(multiplication_table[j]);
    quotient.push_back(j + '0');
    carry_down_res.push_back(current_digits);
    current_digits = subtract(current_digits, multiplication_table[j]);
  }

  for (std::size_t i = 0; i < things_to_subtract.size() - 1; ++i) {
    if (things_to_subtract[i] == things_to_subtract[i + 1] && things_to_subtract[i] == "0") {
      things_to_subtract.erase(things_to_subtract.begin() + i);
      carry_down_res.erase(carry_down_res.begin() + i);
      --i;
    }
  }

  std::cout << "\n";

  // Print the steps
  std::cout << " " << std::string(divisor.length(), ' ') << "     " << quotient
            << "\n";
  std::cout << "  " << std::string(divisor.length(), ' ') << "  "
            << std::string(quotient.length() + 2, '-') << "\n";
  std::cout << " " << divisor << "  |  " << dividend << "\n";

  std::size_t space_to_leave = divisor.length() + 6;
  std::size_t subtraction_bar_len = divisor.length();
  std::cout << std::string(space_to_leave - 3, '-') << " - "
            << std::string(
                   std::max(0UL, (unsigned long)(dividend.length() - things_to_subtract[0].length())),
                   ' ')
            << things_to_subtract[0] << "\n";

  for (std::size_t i = 1; i < things_to_subtract.size(); ++i) {
    if (i != 1) {
      subtraction_bar_len =
          std::max(things_to_subtract[i].length(), carry_down_res[i].length());
    }

    std::cout << std::string(space_to_leave, ' ')
              << std::string(subtraction_bar_len, '-') << "\n";

    space_to_leave += std::abs((long)carry_down_res[i - 1].length() -
                               ((long)carry_down_res[i].length() - 1));

    std::cout << std::string(space_to_leave, ' ') << carry_down_res[i] << "\n";
    std::cout << std::string(space_to_leave - 3, ' ') << " - "
              << std::string(std::max(0UL, (unsigned long)(carry_down_res[i].length() -
                                               things_to_subtract[i].length())),
                             ' ')
              << things_to_subtract[i] << "\n";
  }
  std::cout << "\n";
}
