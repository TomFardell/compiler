#include "emitter.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "ast.hpp"

std::string Emitter::get_code_from_ast_node(ASTNode &node) { return ""; }

void Emitter::emit_program(ASTNode &program_node) {
  if (program_node.type != AST_NODE_PROGRAM) {
    std::cerr << "Emitter received ASTNode of incorrect type\n";
    std::exit(EXIT_FAILURE);
  }

  std::ofstream out_file{m_out_path};
  out_file << get_code_from_ast_node(program_node);
  out_file.close();
}
