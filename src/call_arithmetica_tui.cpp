extern int arithmetica_tui(int argc, char **argv);

#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

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
  std::stringstream s;
  auto buf = std::cout.rdbuf();
  std::cout.rdbuf(s.rdbuf());

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return nullptr;
  }

  int stdinfd = dup(fileno(stdin));
  if (stdinfd == -1) {
    perror("dup");
    close(pipefd[0]);
    close(pipefd[1]);
    return nullptr;
  }

  if (dup2(pipefd[0], fileno(stdin)) == -1) {
    perror("dup2");
    close(pipefd[0]);
    close(pipefd[1]);
    return nullptr;
  }

  FILE *pipeWrite = fdopen(pipefd[1], "w");
  if (pipeWrite == nullptr) {
    perror("fdopen");
    close(pipefd[0]);
    close(pipefd[1]);
    return nullptr;
  }
  fprintf(pipeWrite, command.c_str());
  fclose(pipeWrite);

  std::vector<std::string> argv_vec = {"arithmetica", "--no-introduction"};

  std::vector<char *> argv_cstr;
  for (const auto &arg : argv_vec) {
    argv_cstr.push_back(const_cast<char *>(arg.c_str()));
  }
  argv_cstr.push_back(nullptr);

  int n =
      arithmetica_tui(static_cast<int>(argv_cstr.size() - 1), argv_cstr.data());

  if (dup2(stdinfd, fileno(stdin)) == -1) {
    std::cerr << "Failed to restore stdin." << std::endl;
    close(pipefd[0]);
    close(pipefd[1]);
    return nullptr;
  }
  close(pipefd[0]);
  close(pipefd[1]);

  std::cout.rdbuf(buf);
  std::string test = s.str();
  test = shorten_console_output(test);

  return test;
}