#ifndef PARSER_H
#define PARSER_H

#include "lexer.hpp"
#include "token.hpp"

class Parser {
 private:
  Lexer &m_lexer;            // Reference to the lexer
  Token m_curr_token;        // Current token parsed
  Token m_next_token;        // Peek at the next token to be parsed
  const bool m_print_debug;  // Whether to print debug messages during parsing

  // Stop the compilation due to a parsing error
  void abort(std::string_view);

 public:
  // Constructor taking a reference to the lexer
  Parser(Lexer &lexer, bool print_debug);
  // Try to match and pass over a token of type `token_type`, aborting if not found
  void match_token(TokenType token_type);
  // Move forwards one token
  void next_token();
};

#endif
