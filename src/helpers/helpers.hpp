#pragma once

#include <arithmetica.hpp>
#include <basic_math_operations.hpp>
#include <cstring>
#include <fstream>
#include <functions.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <termios.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

int get_console_width();
void replace_all(std::string &str, const std::string &from,
                 const std::string &to);
bool compare_text_without_ansi(std::string a, std::string b);
int compare_printed_text(const std::string &a, const std::string &b);
std::string round_decimal(std::string decimal, int n);
std::string center(std::string str, size_t n);
std::string remove_extra_front_back_brackets(std::string str);
size_t get_matching_brace(std::string str, size_t index);
std::vector<std::string> get_printable_result(std::string str);
void print_result(std::string str, std::ostream &outstream = std::cout);
void print_expression(std::vector<std::string> terms,
                      std::vector<std::string> signs, int padding,
                      std::ofstream *file = NULL,
                      std::vector<std::string> *out = NULL,
                      size_t padding_exclude = 0);
void print_eval_expression(std::string expression, int outputType, int padding,
                           std::vector<std::string> *outTerms = NULL,
                           std::vector<std::string> *outSigns = NULL,
                           std::ostream &outstream = std::cout);
std::vector<std::string> tokenize(std::string s);
bool check_for_implicit_eval(std::string &s);

#ifdef __linux__
char getch(std::istream &instream);
#endif