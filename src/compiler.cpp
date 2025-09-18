#include "lexer.hpp"
#include "parser.hpp"

int main() {
  Lexer lexer{"int x; int main(void) { int x; if (!0) x = 2; }"};
  Parser parser{lexer, true};

  parser.parse();

  return 0;
}
