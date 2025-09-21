#ifndef TOKEN_H
#define TOKEN_H

#include <string_view>
#include <unordered_map>

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
  TOKEN_VOID,
  TOKEN_WHILE,
  TOKEN_WRITE,

  // Operators and symbols
  TOKEN_AND,        // Operator: &&
  TOKEN_ASSIGN,     // Operator: =
  TOKEN_COMMA,      // Operator: ,
  TOKEN_DIVIDE,     // Operator: /
  TOKEN_EQ,         // Operator: ==
  TOKEN_GE,         // Operator: >=
  TOKEN_GT,         // Operator: >
  TOKEN_LBRACE,     // Symbol:   {
  TOKEN_LBRACKET,   // Symbol:   [
  TOKEN_LE,         // Operator: <=
  TOKEN_LPAREN,     // Symbol:   (
  TOKEN_LT,         // Operator: <
  TOKEN_MINUS,      // Operator: -
  TOKEN_MULTIPLY,   // Operator: *
  TOKEN_NEQ,        // Operator: !=
  TOKEN_NOT,        // Operator: !
  TOKEN_OR,         // Operator: ||
  TOKEN_PLUS,       // Operator: +
  TOKEN_RBRACE,     // Symbol:   }
  TOKEN_RBRACKET,   // Symbol:   ]
  TOKEN_RPAREN,     // Symbol:   )
  TOKEN_SEMICOLON,  // Symbol:   ;

};

class Token {
 private:
  std::string_view m_text;  // Text associated with the token
  TokenType m_type;         // Type of the token

 public:
  // Constructor with token text and type
  Token(std::string_view text, TokenType type) : m_text{text}, m_type{type} {};

  // Get the text associated with the token
  std::string_view get_text() { return m_text; }
  // Get the type of the token
  TokenType get_type() { return m_type; }

  // TokenType lookup for keyword strings
  inline static const std::unordered_map<std::string_view, TokenType> keywords{
      {"else", TOKEN_ELSE},   {"exit", TOKEN_EXIT},  {"float", TOKEN_FLOAT},   {"if", TOKEN_IF},
      {"int", TOKEN_INT},     {"read", TOKEN_READ},  {"return", TOKEN_RETURN}, {"void", TOKEN_VOID},
      {"while", TOKEN_WHILE}, {"write", TOKEN_WRITE}};

  // Name lookup for the enum
  inline static const std::unordered_map<TokenType, std::string_view> type_names{
      {TOKEN_NULL, "null"},
      {TOKEN_EOF, "eof"},
      {TOKEN_IDENTIFIER, "identifier"},
      {TOKEN_INT_LITERAL, "int literal"},
      {TOKEN_FLOAT_LITERAL, "float literal"},
      {TOKEN_STRING_LITERAL, "string literal"},
      {TOKEN_ELSE, "else"},
      {TOKEN_EXIT, "exit"},
      {TOKEN_FLOAT, "float"},
      {TOKEN_IF, "if"},
      {TOKEN_INT, "int"},
      {TOKEN_READ, "read"},
      {TOKEN_RETURN, "return"},
      {TOKEN_VOID, "void"},
      {TOKEN_WHILE, "while"},
      {TOKEN_WRITE, "write"},
      {TOKEN_AND, "and"},
      {TOKEN_ASSIGN, "assign"},
      {TOKEN_COMMA, "comma"},
      {TOKEN_DIVIDE, "divide"},
      {TOKEN_EQ, "eq"},
      {TOKEN_GE, "ge"},
      {TOKEN_GT, "gt"},
      {TOKEN_LBRACE, "lbrace"},
      {TOKEN_LBRACKET, "lbracket"},
      {TOKEN_LE, "le"},
      {TOKEN_LPAREN, "lparen"},
      {TOKEN_LT, "lt"},
      {TOKEN_MINUS, "minus"},
      {TOKEN_MULTIPLY, "multiply"},
      {TOKEN_NEQ, "neq"},
      {TOKEN_NOT, "not"},
      {TOKEN_OR, "or"},
      {TOKEN_PLUS, "plus"},
      {TOKEN_RBRACE, "rbrace"},
      {TOKEN_RBRACKET, "rbracket"},
      {TOKEN_RPAREN, "rparen"},
      {TOKEN_SEMICOLON, "semicolon"},
  };
};

#endif
