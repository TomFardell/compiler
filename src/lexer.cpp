#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include "lexer.hpp"
#include "token.hpp"

Lexer::Lexer(std::string_view source)
    : m_source{source},
      m_source_length{static_cast<int>(std::size(m_source))},
      m_cursor_char{' '},
      m_cursor_pos{-1} {
  next_char();
}

char Lexer::peek() {
  if (m_cursor_pos + 1 >= m_source_length)
    return '\0';
  else
    return m_source[m_cursor_pos + 1];
}

void Lexer::next_char() {
  ++m_cursor_pos;

  if (m_cursor_pos >= m_source_length)
    m_cursor_char = '\0';
  else
    m_cursor_char = m_source[m_cursor_pos];
}

void Lexer::skip_whitespace() {
  while (m_cursor_char == ' ' || m_cursor_char == '\t' || m_cursor_char == '\n') next_char();
}

void Lexer::skip_comments() {
  if (m_cursor_char == '/' && peek() == '*') {
    // Note that C-- does not allow nested comment blocks
    while (!(m_cursor_char == '*' && peek() == '/')) next_char();

    // Skip the remaining '*' and '/'
    next_char();
    next_char();

    // We have found a comment, so skip whitespace and check for another comment
    skip_whitespace();
    skip_comments();
  }
}

Token Lexer::get_token() {
  skip_whitespace();
  skip_comments();
  skip_whitespace();
  Token result{"", TOKEN_NULL};

  // Single character tokens
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
  // (Potential) double character tokens
  else if (m_cursor_char == '&') {
    if (peek() == '&') {
      result = Token{"&&", TOKEN_AND};
      next_char();
    } else {
      abort(std::format("Invalid token '&{}'", peek()));
    }
  } else if (m_cursor_char == '=') {
    if (peek() == '=') {
      result = Token{"==", TOKEN_EQ};
      next_char();
    } else {
      result = Token{"=", TOKEN_ASSIGN};
    }
  } else if (m_cursor_char == '>') {
    if (peek() == '=') {
      result = Token{">=", TOKEN_GE};
      next_char();
    } else {
      result = Token{">", TOKEN_GT};
    }
  } else if (m_cursor_char == '<') {
    if (peek() == '=') {
      result = Token{"<=", TOKEN_LE};
      next_char();
    } else {
      result = Token{"<", TOKEN_LT};
    }
  } else if (m_cursor_char == '!') {
    if (peek() == '=') {
      result = Token{"!=", TOKEN_NEQ};
      next_char();
    } else {
      abort(std::format("Invalid token '!{}", peek()));
    }
  } else if (m_cursor_char == '|') {
    if (peek() == '|') {
      result = Token{"||", TOKEN_OR};
      next_char();
    } else {
      abort(std::format("Invalid token '|{}'", peek()));
    }
  }

  next_char();
  return result;
}

void Lexer::abort(std::string message) {
  std::cout << "Compilation aborted: lexer error\n" << message << "\n";
  exit(EXIT_FAILURE);
}
