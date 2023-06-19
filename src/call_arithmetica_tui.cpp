#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

int arithmetica_tui(int argc, char **argv, std::istream &instream_ = std::cin,
                    std::ostream &outstream_ = std::cout);

void insert_characters_into_line(std::string &line, std::string chars,
                                 int &cursor_location) {
  auto i = cursor_location;
  auto _cursor_location = cursor_location;
  for (; i < _cursor_location + chars.length() && i < line.length(); ++i) {
    line[i] = chars[i - cursor_location];
    ++cursor_location;
  }
  if (i < _cursor_location + chars.length()) {
    std::string substr = chars.substr(i - _cursor_location);
    line += substr;
    cursor_location = line.length();
  }
}

std::string shorten_console_output(std::string str) {
  std::string answer;
  std::string current_line;

  int cursor_location = 0;

  for (auto &i : str) {
    if (current_line.length() >= 2 &&
        current_line.substr(current_line.length() - 2, 2) == "\\r") {
      cursor_location = 0;
      current_line = current_line.substr(0, current_line.length() - 2);
    }
    if (current_line.length() >= 7 &&
        current_line.substr(current_line.length() - 7, 7) == "\\033[2K") {
      current_line.clear();
    }

    if (i == '\033') {
      insert_characters_into_line(current_line, "\\033", cursor_location);
      continue;
    }
    if (i == '\r') {
      insert_characters_into_line(current_line, "\\r", cursor_location);
      continue;
    }
    if (i == '\n') {
      answer += current_line + "\\n";
      current_line.clear();
      cursor_location = 0;
      continue;
    }

    insert_characters_into_line(current_line, std::string(1, i),
                                cursor_location);
  }

  return answer;
}

std::string call_arithmetica_tui(std::string command) {
  command += "\nexit\n";

  std::vector<std::string> argv_vec = {"arithmetica", "--no-introduction"};

#ifdef _WIN32
  argv_vec.push_back("--reprint-input");
#endif

  std::vector<char *> argv_cstr;
  for (const auto &arg : argv_vec) {
    argv_cstr.push_back(const_cast<char *>(arg.c_str()));
  }
  argv_cstr.push_back(nullptr);

  std::stringstream in, out;
  in << command;
  int n =
      arithmetica_tui(static_cast<int>(argv_cstr.size() - 1), argv_cstr.data(), in, out);

  std::string answer = out.str();
  answer = shorten_console_output(answer);

  return answer;
}