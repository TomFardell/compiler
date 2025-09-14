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

  // Get whether a character is alphabetical
  static bool is_alpha(char c) { return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'); }
  // Get whether a character is a digit
  static bool is_digit(char c) { return '0' <= c && c <= '9'; }
  // Get whether a character is valid in an identifier name (following the first character which must be a letter)
  static bool is_id_char(char c) { return is_alpha(c) || is_digit(c) || (c == '_'); }
  // Get whether a character is a whitespace character
  static bool is_whitespace(char c) { return (c == ' ') || (c == '\n') || (c == '\t'); }
  // Stop the compilation due to a lexing error
  void abort(std::string_view);

 public:
  // Constructor given view of source code
  Lexer(std::string_view source);
  // Get the character under the cursor
  char get_cursor_char() { return m_cursor_char; }
  // Look ahead at the next character in the source string without processing
  char peek();

  // Move the cursor forwards one character
  void next_char();
  // Move the cursor over any whitespace characters
  void skip_whitespace();
  // Move the cursor past a comment, returning whether a comment was found
  bool skip_comment();
  // Move the cursor past any blocks of whitespace and comments
  void skip_whitespace_and_comments();
  // Get the next token from the source string
  Token get_token();
};

#endif
