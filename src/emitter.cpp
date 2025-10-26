#include "emitter.hpp"

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>

#include "ast.hpp"

std::string Emitter::process_ast_node(ASTNode &node) {
  switch (node.type) {
    /*-----------*/
    /* Null node */
    /*-----------*/
    case AST_NODE_NULL: {
      std::cerr << "Cannot emit code from null node\n";
      exit(EXIT_FAILURE);
    }

    /*--------------*/
    /* Program node */
    /*--------------*/
    case AST_NODE_PROGRAM: {
      std::string result{};

      result.append("global main\n");
      result.append("\n");
      result.append("extern printf\n");
      result.append("\n");
      result.append("section .data:\n");
      result.append("  int_fmt: db \"%d\", 0xA, 0x0\n");  // Format to print integers using printf
      result.append("  flt_fmt: db \"%f\", 0xA, 0x0\n");  // Format to print floats using printf

      // Formats to print literal strings using printf
      for (auto const &[i, string_literal] : std::views::enumerate(m_string_literals)) {
        result.append(
            std::format("  {}{}: db \"{}\", 0xA, 0x0\n", m_string_literal_identifier, i, string_literal));
      }

      result.append("\n");
      result.append("section .text:");

      // Add the code from this node's children
      for (ASTNode &child_node : node.children) {
        result.append(process_ast_node(child_node));
      }

      return result;
    }

    // TODO: Remove once all cases filled out
    default: {
      return "";
    }
  }
}

void Emitter::emit_program(ASTNode &program_node) {
  if (program_node.type != AST_NODE_PROGRAM) {
    std::cerr << "Emitter received ASTNode of incorrect type\n";
    std::exit(EXIT_FAILURE);
  }

  std::ofstream out_file{m_out_path};
  out_file << process_ast_node(program_node);
  out_file.close();
}
