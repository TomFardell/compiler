#ifndef TOKEN_H
#define TOKEN_H

#include <string_view>

enum TokenType {
  TOKEN_NULL,
  TOKEN_EOF,
  TOKEN_IDENTIFIER,
  TOKEN_INT_LITERAL,
  TOKEN_FLOAT_LITERAL,

  // Keywords
  TOKEN_ELSE,
  TOKEN_EXIT,
  TOKEN_FLOAT,
  TOKEN_IF,
  TOKEN_INT,
  TOKEN_READ,
  TOKEN_RETURN,
  TOKEN_WHILE,
  TOKEN_WRITE,

  // Operators
  TOKEN_AND,        // Operator: &&
  TOKEN_ASSIGN,     // Operator: =
  TOKEN_COMMA,      // Operator: ,
  TOKEN_DIVIDE,     // Operator: /
  TOKEN_EQ,         // Operator: ==
  TOKEN_GE,         // Operator: >=
  TOKEN_GT,         // Operator: >
  TOKEN_LBRACE,     // Operator: {
  TOKEN_LBRACKET,   // Operator: [
  TOKEN_LE,         // Operator: <=
  TOKEN_LPAREN,     // Operator: (
  TOKEN_LT,         // Operator: <
  TOKEN_MINUS,      // Operator: -
  TOKEN_MULTIPLY,   // Operator: *
  TOKEN_NEQ,        // Operator: !=
  TOKEN_OR,         // Operator: ||
  TOKEN_PLUS,       // Operator: +
  TOKEN_QUOTE,      // Operator: '
  TOKEN_RBRACE,     // Operator: }
  TOKEN_RBRACKET,   // Operator: ]
  TOKEN_RPAREN,     // Operator: )
  TOKEN_SEMICOLON,  // Operator: ;

};

class Token {
 private:
  std::string_view m_text;
  TokenType m_type;

 public:
  Token(std::string_view text, TokenType type) : m_text{text}, m_type{type} {};
  std::string_view get_text() { return m_text; }
  TokenType get_type() { return m_type; }
};

#endif
