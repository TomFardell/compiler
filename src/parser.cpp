#include <format>
#include <iostream>
#include "parser.hpp"

void Parser::abort(std::string_view message) {
  std::cout << "Compilation aborted: parser error\n-> " << message << "\n";
  exit(EXIT_FAILURE);
}

Parser::Parser(Lexer &lexer, bool print_debug)
    : m_lexer{lexer},
      m_curr_token{lexer.get_token()},
      m_next_token{lexer.get_token()},
      m_print_debug{print_debug} {};

void Parser::match_token(TokenType token_type) {
  // If the token doesn't match the expected type, abort
  if (m_curr_token.get_type() != token_type) {
    abort(std::format("Unexpected token of type '{}'", static_cast<int>(m_curr_token.get_type())));
  }
  // Otherwise, move to the next token
  next_token();
}

void Parser::next_token() {
  m_curr_token = m_next_token;
  m_next_token = m_lexer.get_token();
}
