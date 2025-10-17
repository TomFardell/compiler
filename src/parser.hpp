#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#include "ast.hpp"
#include "emitter.hpp"
#include "lexer.hpp"
#include "token.hpp"

class Parser {
 private:
  Lexer &m_lexer;      // Reference to the lexer
  Emitter &m_emitter;  // Reference to the emitter

  std::vector<Token> m_tokens;  // Vector of tokens
  int m_cursor_pos;             // Position of the cursor through the vector of tokens
  const bool m_print_debug;     // Whether to print debug messages during parsing

  ASTNode m_ast_root;  // Root node of the generated AST

  /*---------*/
  /* Grammar */
  /*-------------------------------------------------------------------------------------------------------------*/

  // Grammar functions work as follows:
  // Each function tries to match one of its rules. It goes through each rule and returns the necessary information
  // for building the AST if the rule matches, moving the cursor forwards through those tokens. If a rule does not
  // match, it moves to the next rule. If no rule matches, that function returns a nulled object (this will depend
  // on the return type) and moves the cursor back to the position it was at on entry to the function. Some
  // functions can abort the parser, but only if they find a sequence of tokens or consecutive rules that in no
  // circumstances would match the grammar

  // Grammar notation in below comments:
  // {}   matches 0 or more of its contents
  // []   matches 0 or 1 of its contents
  // ()   is just for grouping
  // |    is the logical or
  // tkn_ denotes a token type
  // ...  continues the statement of the previous line

  // prog: {(decl tkn_semi) | func}
  ASTNode program();

  // decl: type tkn_id {tkn_comma tkn_id}
  //     | (type | tkn_void) tkn_id tkn_lparen param_types
  //       ... tkn_rparen {tkn_comma tkn_id tkn_lparen param_types tkn_rparen}
  std::vector<ASTNode> declaration();

  // param_types: tkn_void
  //            | type tkn_id {tkn_comma type tkn_id}
  std::vector<ASTNode> parameter_types();

  // func: (type | tkn_void) tkn_id tkn_lparen param_types tkn_rparen
  //       ... tkn_lbrace {type tkn_id {tkn_comma tkn_id} tkn_semi} {stmnt} tkn_rbrace
  ASTNode function();

  // type: tkn_flt
  //     | tkn_int
  std::string type();

  // stmnt: tkn_if tkn_lparen expr tkn_rparen stmt [tkn_else stmt]
  //      | tkn_while tkn_lparen expr tkn_rparen stmt
  //      | tkn_return [expr] tkn_semi
  //      | tkn_read tkn_lparen tk_id tkn_rparen tkn_semi
  //      | tkn_write tkn_lparen (tkn_str_lit | expr) tkn_rparen tkn_semi
  //      | tkn_id tkn_lparen [expr {tkn_comma expr}] tkn_rparen tkn_semi
  //      | tkn_id tkn_assgn expr tk_semi
  //      | tkn_lbrace {stmt} tkn_rbrace
  //      | tkn_semi
  ASTNode statement();

  // expr: tkn_lparen expr tk_rparen
  //     | tkn_min expr
  //     | tkn_not expr
  //     | tkn_id [tkn_lparen [expr {tkn_comma expr}] tkn_rparen]
  //     | expr bin_op expr
  //     | expr rel_op expr
  //     | expr log_op expr
  //     | tkn_float_lit
  //     | tkn_int_lit
  //     | tkn_str_lit
  ASTNode expression();

  // bin_op: tkn_plus | tkn_min | tkn_mul | tkn_div | tkn_eq | tkn_neq | tkn_lt | tkn_le | tkn_gt | tkn_ge
  //         ... | tkn_and | tkn_or
  std::string binary_operator();

  // Read one token of the given type
  bool token(TokenType token_type);

  /*-------------------------------------------------------------------------------------------------------------*/

  // Stop the compilation due to a parsing error
  void abort(std::string_view);

 public:
  // Constructor taking a reference to the lexer and emitter, and bool for whether to print debug text
  Parser(Lexer &lexer, Emitter &emitter, bool print_debug);
  // Move the cursor forwards by one token
  void next_token();
  // Move cursor to the given index
  void move_cursor_back_to(int idx);

  // Parse all tokens and write to file
  void parse();
};

#endif
