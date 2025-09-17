#include <iostream>
#include "token.hpp"
#include "parser.hpp"

bool Parser::program() {
  while (!check_cursor_token(TOKEN_EOF)) {
    if (declaration()) {
      if (check_cursor_token(TOKEN_SEMICOLON)) {
        next_token();  // Read the semicolon
        continue;
      }

      abort("Expected ';' after declaration");
    } else if (function()) {
      continue;
    }
    return false;
  }

  return true;
}

void Parser::abort(std::string_view message) {
  std::cout << "Compilation aborted: parser error\n-> " << message << "\n";
  exit(EXIT_FAILURE);
}

Parser::Parser(Lexer &lexer, bool print_debug)
    : m_lexer{lexer}, m_tokens{}, m_cursor_pos{-1}, m_print_debug{print_debug} {};

void Parser::next_token() {
  ++m_cursor_pos;

  // If the new cursor position goes past the stored tokens, read another
  if (m_cursor_pos >= static_cast<int>(m_tokens.size())) {
    m_tokens.emplace_back(m_lexer.get_token());
  }
}

void Parser::move_cursor(int idx) {
  if (idx > m_cursor_pos) {  // If the index is ahead, read off tokens until we reach it
    while (idx > m_cursor_pos) next_token();
  } else {  // Otherwise just move the cursor back
    m_cursor_pos = idx;
  }
}
