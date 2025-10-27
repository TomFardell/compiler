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
      abort("Cannot emit code from null node");
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
      result.append("section .data\n");
      result.append("  int_fmt: db \"%d\", 0xA, 0x0\n");  // Format to print integers using printf
      result.append("  flt_fmt: db \"%f\", 0xA, 0x0\n");  // Format to print floats using printf

      // Formats to print literal strings using printf
      for (auto const &[i, string_literal] : std::views::enumerate(m_string_literals)) {
        result.append(
            std::format("  {}{}: db \"{}\", 0xA, 0x0\n", m_string_literal_identifier, i, string_literal));
      }

      std::string bss_section{};
      std::string text_section{};

      for (ASTNode &child_node : node.children) {
        if (child_node.type == AST_NODE_VARIABLE_DECLARATION)
          bss_section.append(process_ast_node(child_node));
        else if (child_node.type == AST_NODE_FUNCTION_DEFINITION)
          text_section.append(process_ast_node(child_node));
        else if (child_node.type == AST_NODE_FUNCTION_DECLARATION)
          process_ast_node(child_node);  // Returns empty string
        else
          abort("Unexpected type of child node of program node");
      }

      result.append("\n");
      result.append("section .bss\n");
      result.append(bss_section);
      result.append("\n");
      result.append("section .text\n");
      result.append(text_section);

      return result;
    }

    /*----------------------*/
    /* Variable declaration */
    /*----------------------*/
    case AST_NODE_VARIABLE_DECLARATION: {
      std::string result{};

      // Scope must be set by the parent node
      if (node.data.contains("scope")) {  // Variables declared in functions will have this set
        FunctionInfo &function_info = m_functions_info.at(node.data.at("scope"));

        function_info.variable_byte_offsets[node.data.at("name")] = function_info.variable_current_offset;
        function_info.variable_current_offset += 4;
      } else {  // Otherwise the declaration has global scope
        if (m_global_variables.contains(node.data.at("name"))) abort("Redeclaration of global variable");

        result.append(std::format("  {}: resb 4\n", node.data.at("name")));
        m_global_variables.insert(node.data.at("name"));
      }

      return result;
    }

    /*----------------------*/
    /* Function declaration */
    /*----------------------*/
    case AST_NODE_FUNCTION_DECLARATION: {
      // If the function was already declared, check the parameters and return type match
      if (m_functions_info.contains(node.data.at("name"))) {
        FunctionInfo &function_info = m_functions_info.at(node.data.at("name"));  // Existing info

        if (function_info.return_type != node.data.at("return type"))
          abort("Redeclaration of function with different return type");

        size_t parameter_count{0};
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;  // The parameter child nodes are at the front
          ++parameter_count;
        }

        if (parameter_count != function_info.parameters.size())
          abort("Redeclaration of function with different number of parameters");

        // Loop through the parameters and check they match the existing definition
        for (auto const &[existing_info, parameter_node] :
             std::views::zip(function_info.parameters, node.children)) {
          if (existing_info.name != parameter_node.data.at("name") ||
              existing_info.type != parameter_node.data.at("type"))
            abort("Redeclaration of function with different parameters");
        }
      } else {  // Otherwise, establish the function info for this declaration
        FunctionInfo &function_info = m_functions_info[node.data.at("name")];

        function_info.return_type = node.data.at("return type");
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;
          function_info.parameters.emplace_back(child_node.data.at("name"), child_node.data.at("type"));
        }
      }

      return "";  // Nothing is actually written to the assembly here
    }

    // TODO: Remove once all cases filled out
    default: {
      return "";
    }
  }
}

void Emitter::abort(std::string_view message) {
  std::cerr << "Compilation aborted: emission error\n-> " << message << "\n";
  std::exit(EXIT_FAILURE);
}

void Emitter::emit_program(ASTNode &program_node) {
  if (program_node.type != AST_NODE_PROGRAM) abort("Received ASTNode of incorrect type");

  std::ofstream out_file{m_out_path};
  out_file << process_ast_node(program_node);
  out_file.close();
}
