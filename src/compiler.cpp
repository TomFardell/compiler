#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <iostream>

int main() {
  Lexer lexer{"+- */ /* comment */  /* Another comment */ =="};
  Parser parser{lexer, true};

  parser.move_cursor(0);
  std::cout << parser.check_cursor_token(TOKEN_PLUS);

  parser.move_cursor(3);
  std::cout << parser.check_cursor_token(TOKEN_DIVIDE);

  parser.move_cursor(2);
  std::cout << parser.check_cursor_token(TOKEN_MULTIPLY) << " = 111\n";

  return 0;
}
