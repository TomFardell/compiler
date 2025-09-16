#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

int main() {
  Lexer lexer{"+- */ /* comment */  /* Another comment */ =="};
  Parser parser{lexer, true};

  // Should run with no parser abort
  parser.match_token(TOKEN_PLUS);
  parser.match_token(TOKEN_MINUS);
  parser.match_token(TOKEN_MULTIPLY);
  parser.match_token(TOKEN_DIVIDE);
  parser.match_token(TOKEN_EQ);

  return 0;
}
