#include "helpers.hpp"

int get_console_width() {
#ifdef __linux__
  int ans = 20;
  struct winsize w;
  memset(&w, 0, sizeof(w));
  w.ws_col = 0;
  w.ws_row = 0;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  if (w.ws_col > 0) {
    ans = w.ws_col;
  }
  return ans;
#endif
#ifdef _WIN32
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#endif
}