#ifndef PARSER_H
#define PARSER_H

#include "lexer.hpp"
#include "token.hpp"
#include <vector>

class Parser {
 private:
  Lexer &m_lexer;               // Reference to the lexer
  std::vector<Token> m_tokens;  // Vector of tokens
  int m_cursor_pos;             // Position of the cursor through the vector of tokens
  const bool m_print_debug;     // Whether to print debug messages during parsing

  /*---------*/
  /* Grammar */
  /*-------------------------------------------------------------------------------------------------------------*/

  // Grammar functions work as follows:
  // Each function tries to match one of its rules. It goes through each rule and returns 'true' if the rule
  // matches, moving the cursor forwards through those tokens. If a rule does not match, it moves to the next rule.
  // If no rule matches, that function returns 'false' and moves the cursor back to the position it was at on entry
  // to the function. Some functions can abort the parser, but only if they find a sequence of tokens or
  // consecutive rules that in no circumstances would match the grammar

  // Grammar notation in below comments:
  // {}   matches 0 or more of its contents
  // []   matches 0 or 1 of its contents
  // ()   is just for grouping
  // |    is the logical or
  // tkn_ denotes a token type
  // ...  continues the statement of the previous line

  // prog: {(decl tkn_semi) | func}
  bool program();

  // decl: type var_decl {tkn_comma var_decl}
  //     | (type | tkn_void) tkn_id tkn_lparen param_types
  //       ... tkn_rparen {tkn_comma tkn_id tkn_lparen param_types tkn_rparen}
  bool declaration();

  // param_types: tkn_void
  //            | type tkn_id [arr_decl_suff] {tkn_comma type tkn_id [arr_decl_suff]}
  bool parameter_types();

  // func: (type | tkn_void) tkn_id tkn_lparen param_types tkn_rparen
  //       ... tkn_lbrace {type var_decl {tkn_comma var_decl} tkn_semi} {stmnt} tkn_rbrace
  bool function();

  // var_decl: tkn_id [arr_decl_suff]
  bool variable_declaration();

  // arr_decl_suff: tkn_lbracket tkn_int_lit tkn_rbracket
  bool array_declaration_suffix();

  // type: tkn_flt
  //     | tkn_int
  bool type();

  // stmnt: tkn_if tkn_lparen expr tkn_rparen stmt [tkn_else stmt]
  //      | tkn_while tkn_lparen expr tkn_rparen stmt
  //      | tkn_return [expr] tkn_semi
  //      | assgn tk_semi
  //      | tkn_id tkn_lparen [expr {tkn_comma expr}] tkn_rparen tkn_semi
  //      | tkn_lbrace stmt tkn_rbrace
  //      | tkn_semi
  bool statement();

  // assgn: tkn_id [tkn_lbracket expr tkn_rbracket] tkn_assgn expr
  bool assignment();

  // expr: tkn_min expr
  //     | tkn_not expr
  //     | expr bin_op expr
  //     | expr rel_op expr
  //     | expr log_op expr
  //     | tkn_id [(tkn_lparen [expr {tkn_comma expr}] tkn_rparen) | (tkn_lbracket expr tkn_rbracket)]
  //     | tkn_lparen expr tk_rparen
  //     | tkn_float_lit
  //     | tkn_int_lit
  //     | tkn_str_lit
  bool expression();

  // bin_op: tkn_plus
  //       | tkn_min
  //       | tkn_mul
  //       | tkn_div
  bool binary_operator();

  // rel_op: tkn_eq
  //       | tkn_neq
  //       | tkn_lt
  //       | tkn_le
  //       | tkn_gt
  //       | tkn_ge
  bool relational_operator();

  // log_op: tkn_and
  //       | tkn_or
  bool logical_operator();

  // Read one token of the given type
  bool token(TokenType token_type);

  /*-------------------------------------------------------------------------------------------------------------*/

  // Stop the compilation due to a parsing error
  void abort(std::string_view);

 public:
  // Constructor taking a reference to the lexer and bool for whether to print debug text
  Parser(Lexer &lexer, bool print_debug);
  // Move the cursor forwards by one token
  void next_token();
  // Move cursor to the given index
  void move_cursor_back_to(int idx);

  // Parse all tokens. For now, just print whether the source code is a valid program
  void parse();
};

#endif
