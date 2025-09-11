#ifndef LEXER_H
#define LEXER_H

#include <string_view>
#include "token.hpp"

class Lexer {
 private:
  const std::string_view m_source;  // View of the source code
  const int m_source_length;        // Number of characters in the source code
  char m_cursor_char;               // Character under the cursor
  int m_cursor_pos;                 // Position of the cursor

 public:
  Lexer(std::string_view source);

  void next_char();  // Process the next character in the source string
  char peek();       // Look ahead at the next character in the source string without processing
  char get_cursor_char() { return m_cursor_char; }  // Get the character under the cursor
  Token get_token();                                // Get the next token from the source string
};
#endif
