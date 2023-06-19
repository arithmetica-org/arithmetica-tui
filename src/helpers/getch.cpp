#include "helpers.hpp"

// Get a single character from the console without echo or buffering
#ifdef __linux__
char getch(std::istream &instream) {
  struct termios oldt, newt;
  memset(&oldt, 0, sizeof(oldt));
  memset(&newt, 0, sizeof(newt));
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  char c = instream.get();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return c;
}
#endif