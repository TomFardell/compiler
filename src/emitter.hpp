#ifndef EMITTER_H
#define EMITTER_H

#include <string>

#include "ast.hpp"

class Emitter {
 private:
  const std::string m_out_path;  // File path of the compiled code

  // Given an abstract syntax tree node, get the assembly code associated with that node. Calling this with a
  // program node will return the entire program in assembly
  std::string get_code_from_ast_node(ASTNode &node);

 public:
  // Constructor taking out file path
  Emitter(const std::string out_path) : m_out_path{out_path} {};

  // Emit the program with the given root node to the outfile
  void emit_program(ASTNode &program_node);
};

#endif
