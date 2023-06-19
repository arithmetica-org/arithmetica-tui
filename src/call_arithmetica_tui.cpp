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
  std::vector<std::string> answer = {""};

  int cursor_location = 0;
  size_t line_index = 0;

  for (auto &i : str) {
    if (answer[line_index].length() >= 2 &&
        answer[line_index].substr(answer[line_index].length() - 2, 2) == "\\r") {
      cursor_location = 0;
      answer[line_index] = answer[line_index].substr(0, answer[line_index].length() - 2);
    }
    if (answer[line_index].length() >= 7 &&
        answer[line_index].substr(answer[line_index].length() - 7, 7) == "\\033[2K") {
      answer[line_index].clear();
    }
    if (answer[line_index].length() >= 6 &&
        answer[line_index].substr(answer[line_index].length() - 6, 6) == "\\033[A") {
      answer[line_index] = answer[line_index].substr(0, answer[line_index].length() - 6);
      if (line_index > 0) {
        --line_index;
      }
    }

    if (i == '\033') {
      insert_characters_into_line(answer[line_index], "\\033", cursor_location);
      continue;
    }
    if (i == '\r') {
      insert_characters_into_line(answer[line_index], "\\r", cursor_location);
      continue;
    }
    if (i == '\n') {
      if (line_index == answer.size() - 1)
        answer.push_back("");
      cursor_location = 0;
      ++line_index;
      continue;
    }

    insert_characters_into_line(answer[line_index], std::string(1, i),
                                cursor_location);
  }

  std::string answer_str;
  for (size_t i = 0; i < answer.size(); ++i) {
    answer_str += answer[i];
    if (i != answer.size() - 1) {
      answer_str += "\\n";
    }
  }
  return answer_str;
}

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

std::string call_arithmetica_tui(std::string command) {
  command += "\nexit\n";

  std::stringstream s;
  auto buf = std::cout.rdbuf();
  std::cout.rdbuf(s.rdbuf());

  int pipefd[2];
#ifdef _WIN32
  if (_pipe(pipefd, 4096, O_BINARY) == -1) {
#else
  if (pipe(pipefd) == -1) {
#endif
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

#ifdef _WIN32
  argv_vec.push_back("--reprint-input");
#endif

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