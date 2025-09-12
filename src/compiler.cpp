#include <iostream>
#include "lexer.hpp"

int main() {
  Lexer lexer{"+- */ == = != > >= < <= ||&&&="};

  while (lexer.get_cursor_char() != '\0') {
    Token this_token{lexer.get_token()};
    std::cout << this_token.get_type() << " '" << this_token.get_text() << "'\n";
  }

  return 0;
}
