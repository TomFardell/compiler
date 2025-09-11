#include <string>
#include "lexer.hpp"
#include "token.hpp"

Lexer::Lexer(std::string_view source)
    : m_source{source},
      m_source_length{static_cast<int>(std::size(m_source))},
      m_cursor_char{' '},
      m_cursor_pos{-1} {
  next_char();
}

void Lexer::next_char() {
  ++m_cursor_pos;

  if (m_cursor_pos >= m_source_length)
    m_cursor_char = '\0';
  else
    m_cursor_char = m_source[m_cursor_pos];
}

char Lexer::peek() {
  if (m_cursor_pos + 1 >= m_source_length)
    return '\0';
  else
    return m_source[m_cursor_pos + 1];
}

Token Lexer::get_token() {
  Token result{"", TOKEN_NULL};

  if (m_cursor_char == '\0') {
    result = Token{"", TOKEN_EOF};
  } else if (m_cursor_char == ',') {
    result = Token{",", TOKEN_COMMA};
  } else if (m_cursor_char == '/') {
    result = Token{"/", TOKEN_DIVIDE};
  } else if (m_cursor_char == '{') {
    result = Token{"{", TOKEN_LBRACE};
  } else if (m_cursor_char == '[') {
    result = Token{"[", TOKEN_LBRACKET};
  } else if (m_cursor_char == '(') {
    result = Token{"(", TOKEN_LPAREN};
  } else if (m_cursor_char == '-') {
    result = Token{"-", TOKEN_MINUS};
  } else if (m_cursor_char == '*') {
    result = Token{"*", TOKEN_MULTIPLY};
  } else if (m_cursor_char == '+') {
    result = Token{"+", TOKEN_PLUS};
  } else if (m_cursor_char == '\'') {
    result = Token{"'", TOKEN_QUOTE};
  } else if (m_cursor_char == '}') {
    result = Token{"}", TOKEN_RBRACE};
  } else if (m_cursor_char == ']') {
    result = Token{"]", TOKEN_RBRACKET};
  } else if (m_cursor_char == ')') {
    result = Token{")", TOKEN_RPAREN};
  } else if (m_cursor_char == ';') {
    result = Token{";", TOKEN_SEMICOLON};
  }
  next_char();
  return result;
}
