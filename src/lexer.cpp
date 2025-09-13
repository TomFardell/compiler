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

bool Lexer::skip_comment() {
  if (m_cursor_char == '/' && peek() == '*') {
    // Note that C-- does not allow nested comment blocks
    while (!(m_cursor_char == '*' && peek() == '/')) next_char();

    // Skip the remaining '*' and '/'
    next_char();
    next_char();

    return true;
  }
  return false;
}

void Lexer::skip_whitespace_and_comments() {
  skip_whitespace();

  // Repeatedly skip a comment and any whitespace following that comment
  while (skip_comment()) skip_whitespace();
}

Token Lexer::get_token() {
  skip_whitespace_and_comments();

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
  // Literals
  else if (m_cursor_char == '\"') {
    int start_pos = m_cursor_pos;

    do {
      next_char();
      if (m_cursor_char == '\n') abort("Newline character in string literal");
    } while (m_cursor_char != '\"');

    result = Token{m_source.substr(start_pos + 1, m_cursor_pos - start_pos - 1), TOKEN_STRING_LITERAL};
  }

  next_char();
  return result;
}

void Lexer::abort(std::string_view message) {
  std::cout << "Compilation aborted: lexer error\n-> " << message << "\n";
  exit(EXIT_FAILURE);
}
