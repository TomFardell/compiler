#ifndef AST_H
#define AST_H

#include <string>
#include <unordered_map>
#include <vector>

enum ASTNodeType {
  AST_NODE_NULL,

  AST_NODE_PROGRAM,

  AST_NODE_VARIABLE_DECLARATION,
  AST_NODE_FUNCTION_DECLARATION,

  AST_NODE_FUNCTION_DEFINITION,

  AST_NODE_PARAMETER,
  AST_NODE_VOID_PARAMETERS,

  AST_NODE_STATEMENT_IF,
  AST_NODE_STATEMENT_WHILE,
  AST_NODE_STATEMENT_RETURN,
  AST_NODE_STATEMENT_READ,
  AST_NODE_STATEMENT_WRITE,
  AST_NODE_STATEMENT_FUNCTION_CALL,
  AST_NODE_STATEMENT_ASSIGNMENT,
  AST_NODE_STATEMENT_LIST,
  AST_NODE_STATEMENT_EMPTY,

  AST_NODE_EXPRESSION_UNARY_OPERATION,
  AST_NODE_EXPRESSION_BINARY_OPERATION,
  AST_NODE_EXPRESSION_VARIABLE,
  AST_NODE_EXPRESSION_FUNCTION_CALL,
  AST_NODE_EXPRESSION_LITERAL,

  AST_NODE_STRING_LITERAL
};

struct ASTNode {
  ASTNodeType type;                                   // Type of this abstract syntax tree node
  std::unordered_map<std::string, std::string> data;  // Data associated with this node. Will vary with the type
  std::vector<ASTNode> children;  // Children of this node. The expected number of children depends on the type

  void print_tree(int indent = 0);  // Print the abstract syntax tree with this node as its root

  // Name lookup for the enum
  inline static const std::unordered_map<ASTNodeType, std::string> type_names{
      {AST_NODE_NULL, "null"},
      {AST_NODE_PROGRAM, "program"},
      {AST_NODE_VARIABLE_DECLARATION, "variable declaration"},
      {AST_NODE_FUNCTION_DECLARATION, "function declaration"},
      {AST_NODE_FUNCTION_DEFINITION, "function definition"},
      {AST_NODE_PARAMETER, "parameter"},
      {AST_NODE_VOID_PARAMETERS, "void parameters"},
      {AST_NODE_STATEMENT_IF, "statement (if)"},
      {AST_NODE_STATEMENT_WHILE, "statement (while)"},
      {AST_NODE_STATEMENT_RETURN, "statement (return)"},
      {AST_NODE_STATEMENT_READ, "statement (read)"},
      {AST_NODE_STATEMENT_WRITE, "statement (write)"},
      {AST_NODE_STATEMENT_FUNCTION_CALL, "statement (function call)"},
      {AST_NODE_STATEMENT_ASSIGNMENT, "statement (assignment)"},
      {AST_NODE_STATEMENT_LIST, "statement (list)"},
      {AST_NODE_STATEMENT_EMPTY, "statement (empty)"},
      {AST_NODE_EXPRESSION_UNARY_OPERATION, "expression (unary operation)"},
      {AST_NODE_EXPRESSION_BINARY_OPERATION, "expression (binary operation)"},
      {AST_NODE_EXPRESSION_VARIABLE, "expression (variable)"},
      {AST_NODE_EXPRESSION_FUNCTION_CALL, "expression (function call)"},
      {AST_NODE_EXPRESSION_LITERAL, "expression (literal)"},
      {AST_NODE_STRING_LITERAL, "string literal"}};
};

#endif
