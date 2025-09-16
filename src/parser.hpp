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

  /*---------*/
  /* Grammar */
  /*--------------------------------------------------------------------------------------------------------------*/

  // Notation for comments underneath:
  // {}   matches 0 or more of its contents
  // []   matches 0 or 1 of its contents
  // ()   is just for grouping
  // |    is the logical or
  // tkn_ denotes a token type
  // ...  continues the statement of the previous line

  // prog: {(decl tkn_semi) | func}
  void program();

  // decl: type var_decl {tkn_comma var_decl}
  //     | (type | tkn_void) id tkn_lparen param_types tkn_rparen {tkn_comma id tkn_lparen param_types tkn_rparen}
  void declaration();

  // var_decl: tkn_id [arr_decl_suff]
  void variable_declaration();

  // type: tkn_flt
  //     | tkn_int
  void type();

  // param_types: void
  //            | type tkn_id [arr_decl_suff] {tkn_comma type tkn_id [arr_decl_suff]}
  void param_types();

  // arr_decl_suff: tkn_lbracket tkn_int_lit tkn_rbracket
  void array_declaration_suffix();

  // func: (type | tkn_void) tkn_id tkn_lparen param_types tkn_rparen
  //       ... tkn_lbrace {type var_decl {tkn_comma var_decl} tkn_semi {stmnt}} tkn_rbrace
  void function();

  // stmnt: tkn_if tkn_lparen expr tkn_rparen [tkn_else stmt]
  //      | tkn_while tkn_lparen expr tkn_rparen stmt
  //      | tkn_return [expr] tkn_semi
  //      | assgn tk_semi
  //      | tkn_id tkn_lparen [expr {tkn_comma expr}] tkn_rparen tkn_semi
  //      | tkn_lbrace stmt tkn_rbrace
  //      | tkn_semi
  void statement();

  // assgn: tkn_id [tkn_lbracket expr tkn_rbracket] tkn_assgn expr
  void assignment();

  // expr: tkn_minus expr
  //     | tkn_not expr
  //     | expr bin_op expr
  //     | expr rel_op expr
  //     | expr log_op expr
  //     | tkn_id [(tkn_lparen [expr {tkn_comma expr}] tkn_rparen) | (tkn_lbracket expr tkn_rbracket)]
  //     | tkn_lparen expr tk_rparen
  //     | tkn_float_lit
  //     | tkn_int_lit
  //     | tkn_str_lit
  void expression();

  // bin_op: tkn_add
  //       | tkn_min
  //       | tkn_mul
  //       | tkn_div
  void binary_operation();

  // rel_op: tkn_eq
  //       | tkn_neq
  //       | tkn_lt
  //       | tkn_le
  //       | tkn_gt
  //       | tkn_ge
  void relational_operation();

  // log_op: tkn_and
  //       | tkn_or
  void logical_operation();

  /*--------------------------------------------------------------------------------------------------------------*/

  // Stop the compilation due to a parsing error
  void abort(std::string_view);

 public:
  // Constructor taking a reference to the lexer and bool for whether to print debug text
  Parser(Lexer &lexer, bool print_debug);
  // Try to match and pass over a token of type `token_type`, aborting if not found
  void match_token(TokenType token_type);
  // Move forwards one token
  void next_token();
};

#endif
