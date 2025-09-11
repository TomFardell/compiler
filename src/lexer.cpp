#include <string>
#include "lexer.hpp"

Lexer::Lexer(std::string source)
    : m_source{source + "\n"},
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
