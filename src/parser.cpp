#include <format>
#include <iostream>
#include "parser.hpp"

void Parser::abort(std::string_view message) {
  std::cout << "Compilation aborted: parser error\n-> " << message << "\n";
  exit(EXIT_FAILURE);
}

Parser::Parser(Lexer &lexer, bool print_debug)
    : m_lexer{lexer}, m_tokens{}, m_cursor_pos{0}, m_print_debug{print_debug} {};

void Parser::match_token(TokenType token_type) {
  // If the token doesn't match the expected type, abort
  if (m_tokens[m_cursor_pos].get_type() != token_type) {
    abort(std::format("Unexpected token of type '{}'", static_cast<int>(m_tokens[m_cursor_pos].get_type())));
  }
  // Otherwise, move to the next token
  next_token();
}

void Parser::next_token() {
  ++m_cursor_pos;

  // If the new cursor position goes past the stored tokens, read another
  if (m_cursor_pos >= static_cast<int>(m_tokens.size())) {
    m_tokens.emplace_back(m_lexer.get_token());
  }
}

Token &Parser::peek_token(int n) {
  // For any indices of tokens that have not been read yet, read them into the vector
  for (int i{static_cast<int>(m_tokens.size())}; i <= m_cursor_pos + n; i++) {
    m_tokens.emplace_back(m_lexer.get_token());
  }

  return m_tokens[m_cursor_pos + n];
}
