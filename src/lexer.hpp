#ifndef LEXER_H
#define LEXER_H

#include <string>
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

  char get_cursor_char() { return m_cursor_char; }  // Get the character under the cursor
  char peek();  // Look ahead at the next character in the source string without processing

  void next_char();                     // Process the next character in the source string
  void skip_whitespace();               // Move the cursor over any whitespace characters
  bool skip_comment();                  // Move the cursor past a comment, returning whether a comment was found
  void skip_whitespace_and_comments();  // Move the cursor past any blocks of whitespace and comments
  Token get_token();                    // Get the next token from the source string

  void abort(std::string);  // Stop the compilation due to a lexing error
};

#endif
