#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <iostream>

int main() {
  Lexer lexer{"+- */ /* comment */  /* Another comment */ =="};
  Parser parser{lexer, true};

  for (int i = 0; i < 10; i++) {
    Token &tkn{parser.peek_token(i)};
    std::cout << tkn.get_type() << ": '" << tkn.get_text() << "'\n";
  }

  // Should run without errors
  parser.match_token(TOKEN_PLUS);
  parser.match_token(TOKEN_MINUS);
  parser.match_token(TOKEN_MULTIPLY);
  parser.match_token(TOKEN_DIVIDE);
  parser.match_token(TOKEN_EQ);
  parser.match_token(TOKEN_EOF);

  return 0;
}
