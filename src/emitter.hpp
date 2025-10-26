#ifndef EMITTER_H
#define EMITTER_H

#include <string>
#include <vector>

#include "ast.hpp"

struct variable {
  std::string type;
  std::string contents;
  std::string scope;
};

class Emitter {
 private:
  const std::string m_out_path;  // File path of the compiled code

  // Given an abstract syntax tree node, get the assembly code associated with that node. Calling this with a
  // program node will return the entire program in assembly. This also fills out any storages of information
  // related to that scope as a whole
  std::string process_ast_node(ASTNode &node);

 public:
  std::vector<std::string> m_string_literals;  // Vector containing all string literals appearing in the program
  std::vector<variable> m_global_variables;    // Vector containing globally defined variables
  std::vector<variable> m_local_variables;     // Vecotr containing locally defined variables

  const std::string m_string_literal_identifier{"str_lit"};  // Name of string literals in the assembly

  // Constructor taking out file path
  Emitter(const std::string out_path) : m_out_path{out_path} {};

  // Emit the program with the given root node to the outfile
  void emit_program(ASTNode &program_node);
};

#endif
