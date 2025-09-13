#ifndef TOKEN_H
#define TOKEN_H

#include <string_view>
#include <map>

enum TokenType {
  TOKEN_NULL,
  TOKEN_EOF,
  TOKEN_IDENTIFIER,
  TOKEN_INT_LITERAL,
  TOKEN_FLOAT_LITERAL,
  TOKEN_STRING_LITERAL,

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
  TOKEN_RBRACE,     // Operator: }
  TOKEN_RBRACKET,   // Operator: ]
  TOKEN_RPAREN,     // Operator: )
  TOKEN_SEMICOLON,  // Operator: ;

};

// TokenType lookup for keyword strings
const std::map<std::string_view, TokenType> token_keyword_lookup = {
    {"else", TOKEN_ELSE},     {"exit", TOKEN_EXIT},   {"float", TOKEN_FLOAT},
    {"if", TOKEN_IF},         {"int", TOKEN_INT},     {"read", TOKEN_READ},
    {"return", TOKEN_RETURN}, {"while", TOKEN_WHILE}, {"write", TOKEN_WRITE}};

class Token {
 private:
  std::string_view m_text;
  TokenType m_type;

 public:
  Token(std::string_view text, TokenType type) : m_text{text}, m_type{type} {};

  std::string_view get_text() { return m_text; }  // Get the text associated with the token
  TokenType get_type() { return m_type; }         // Get the type of the token
};

#endif
