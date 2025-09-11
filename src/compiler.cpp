#include <iostream>
#include "lexer.hpp"

int main() {
  Lexer lexer{"+- */"};

  while (lexer.peek() != '\0') {
    std::cout << lexer.get_cursor_char();
    lexer.next_char();
  }

  std::cout << "\n";

  return 0;
}
