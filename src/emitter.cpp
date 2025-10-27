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
      std::string variable_name{node.data.at("name")};

      // Scope must be set by the parent node
      if (node.data.contains("scope")) {  // Variables declared in functions will have this set
        FunctionInfo &function_info = m_functions_info.at(node.data.at("scope"));
        if (function_info.local_variables.contains(variable_name)) abort("Redeclaration of local variable");

        LocalVariable &variable = function_info.local_variables[variable_name];  // Add to the map

        variable.type = node.data.at("type");
        variable.offset = function_info.variable_current_offset;
        function_info.variable_current_offset += 4;  // Both floats and ints are 4 bytes wide
      } else {  // Otherwise the declaration has global scope. Allocate globals in the .bss section
        if (m_global_variables.contains(variable_name)) abort("Redeclaration of global variable");

        result.append(std::format("  {}: resb 4\n", variable_name));
        m_global_variables[variable_name] = node.data.at("type");
      }

      return result;  // In the local variable case, nothing is added to the assembly
    }

    /*----------------------*/
    /* Function declaration */
    /*----------------------*/
    case AST_NODE_FUNCTION_DECLARATION: {
      std::string function_name{node.data.at("name")};

      // If the function was already declared, check the parameters and return type match
      if (m_functions_info.contains(function_name)) {
        check_function_node_matches_info(node, m_functions_info.at(function_name));
      } else {  // Otherwise, establish the function info for this declaration
        FunctionInfo &function_info = m_functions_info[function_name];  // Zero initialise function info

        function_info.return_type = node.data.at("return type");
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;
          function_info.parameters.emplace_back(child_node.data.at("name"), child_node.data.at("type"));
        }
      }

      return "";  // Nothing is actually added to the assembly here
    }

    /*---------------------*/
    /* Function definition */
    /*---------------------*/
    case AST_NODE_FUNCTION_DEFINITION: {
      std::string result{};
      std::string function_name{node.data.at("name")};

      if (m_functions_info.contains(function_name)) {
        FunctionInfo &function_info = m_functions_info.at(function_name);
        if (function_info.defined) abort("Redefinition of function");
        check_function_node_matches_info(node, function_info);
      } else {
        FunctionInfo &function_info = m_functions_info[function_name];  // Zero initialise function info

        function_info.return_type = node.data.at("return type");
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;
          function_info.parameters.emplace_back(child_node.data.at("name"), child_node.data.at("type"));
        }
        function_info.defined = true;
      }

      result.append(std::format("{}:\n", function_name));
      result.append("  push rbp\n");
      result.append("  mov rbp, rsp\n");

      // TODO: Push any callee-saved registers that end up being used
      // TODO: Push parameters to the stack here

      std::string body{};

      for (ASTNode &child_node : node.children) {
        if (child_node.type == AST_NODE_VARIABLE_DECLARATION) child_node.data["scope"] = function_name;
        body.append(process_ast_node(child_node));
      }

      result.append(std::format("  sub rsp, {}", m_functions_info[function_name].variable_current_offset));
      result.append("\n");

      result.append(body);

      result.append("\n");
      result.append("  mov rsp, rbp\n");
      result.append("  pop rbp\n");
      result.append("  ret\n");

      return result;
    }

    // TODO: Remove once all cases filled out
    default: {
      return "";
    }
  }
}

void Emitter::check_function_node_matches_info(ASTNode &function_node, FunctionInfo &function_info) {
  if (function_info.return_type != function_node.data.at("return type"))
    abort("Redeclaration of function with different return type");

  size_t parameter_count{0};
  for (const ASTNode &child_node : function_node.children) {
    if (child_node.type != AST_NODE_PARAMETER) break;  // The parameter child nodes are at the front
    ++parameter_count;
  }

  if (parameter_count != function_info.parameters.size())
    abort("Redeclaration of function with different number of parameters");

  // Loop through the parameters and check they match the existing definition
  for (auto const &[existing_info, parameter_node] :
       std::views::zip(function_info.parameters, function_node.children)) {
    if (existing_info.name != parameter_node.data.at("name") ||
        existing_info.type != parameter_node.data.at("type"))
      abort("Redeclaration of function with different parameters");
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
