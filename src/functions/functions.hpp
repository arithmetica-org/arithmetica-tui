#pragma once

#include <string>
#include <vector>

void divide_with_steps(const std::string &dividend, const std::string &divisor,
                       std::size_t accuracy);

namespace eval_with_steps {
char *get_numerical_arguments(const char *expression, bool fromLeft,
                              long *signIndexIn, int outputType);

char *simplify_arithmetic_expression(const char *expression_in, int outputType,
                                     size_t accuracy,
                                     std::vector<std::string> &steps,
                                     bool verbose);

size_t get_corresponding_closing_bracket(const char *str, size_t index);
long find_operational_sign(const char *expression, char sign);
size_t get_back_corresponding_bracket(const char *str, size_t index);
}; // namespace eval_with_steps