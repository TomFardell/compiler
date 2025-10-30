#include "emitter.hpp"

#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>

#include "ast.hpp"

void FunctionInfo::add_local_variable(std::string name, std::string type) {
  m_local_variables[name] = {type, m_stack_offset};
  m_stack_offset += 4;  // Both int and float are of size 4
}

void FunctionInfo::add_parameter(std::string name, std::string type) {
  m_parameters.push_back(name);    // Store the parameter's name and position
  add_local_variable(name, type);  // Parameters become local variables
}

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
        result.append(std::format("  {}{}: db \"{}\", 0xA, 0x0\n", string_literal_id, i, string_literal));
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

    /*-----------------------------*/
    /* Global variable declaration */
    /*-----------------------------*/
    case AST_NODE_VARIABLE_DECLARATION: {
      std::string result{};
      std::string variable_name{node.data.at("name")};

      if (m_global_variables.contains(variable_name)) abort("Redeclaration of global variable");

      // The reason for prefixing global variables is to protect against variables with register names
      result.append(std::format("  {}{}: resb 4\n", global_id_prefix, variable_name));
      m_global_variables[variable_name] = node.data.at("type");

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

        function_info.m_return_type = node.data.at("return type");
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;
          function_info.add_parameter(child_node.data.at("name"), child_node.data.at("type"));
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
        if (function_info.m_is_defined) abort("Redefinition of function");
        check_function_node_matches_info(node, function_info);
      } else {
        FunctionInfo &function_info = m_functions_info[function_name];  // Zero initialise function info

        function_info.m_return_type = node.data.at("return type");
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;
          function_info.add_parameter(child_node.data.at("name"), child_node.data.at("type"));
        }

        function_info.m_is_defined = true;
      }

      result.append(std::format("{}:\n", function_name));
      result.append("  push rbp\n");
      result.append("  mov rbp, rsp\n");

      // TODO: Push any callee-saved registers that end up being used

      std::string body{};

      for (ASTNode &child_node : node.children) {
        body.append(process_ast_node(child_node, function_name));
      }

      result.append(std::format("  sub rsp, {}\n", m_functions_info[function_name].m_stack_offset));
      result.append("\n");

      result.append(body);

      result.append("\n");
      result.append("  mov rax, 0\n");                           // If the function exits naturally, return 0
      result.append(std::format(".{}:\n", function_end_label));  // Return statements set rax and jump here
      result.append("  mov rsp, rbp\n");
      result.append("  pop rbp\n");
      result.append("  ret\n");
      result.append("\n");

      return result;
    }

    default: {
      abort("Node requires function name to process");
      return "";  // Never called
    }
  }
}

std::string Emitter::process_ast_node(ASTNode &node, std::string function_name) {
  switch (node.type) {
    /*----------------------------*/
    /* Local variable declaration */
    /*----------------------------*/
    case AST_NODE_VARIABLE_DECLARATION: {
      std::string result{};
      std::string variable_name{node.data.at("name")};

      FunctionInfo &function_info = m_functions_info.at(function_name);

      if (function_info.m_local_variables.contains(variable_name)) abort("Redeclaration of local variable");

      function_info.add_local_variable(variable_name, node.data.at("type"));

      return result;  // In the local variable case, nothing is added to the assembly
    }
    /*-----------*/
    /* Parameter */
    /*-----------*/
    case AST_NODE_PARAMETER: {
      // Parameters are handled in the function definition/declaration so do nothing
      return "";
    }

    /*-----------------*/
    /* Void parameters */
    /*-----------------*/
    case AST_NODE_VOID_PARAMETERS: {
      // Parameters are handled in the function definition/declaration so do nothing
      return "";
    }

    /*--------------*/
    /* If statement */
    /*--------------*/
    case AST_NODE_STATEMENT_IF: {
      std::string result{};

      int if_number{m_functions_info[function_name].m_if_statement_count++};
      bool else_is_present{node.children.size() == 3};

      ASTNode &expression_node = node.children[0];
      ASTNode &true_statement_node = node.children[1];

      result.append(process_ast_node(expression_node, function_name));

      // Comparisons always evaluate to zero or one at the expression level, meaning any if statement is just a
      // check that the result of the comparison isn't zero (so non-boolean values are treated as true if they are
      // not zero)
      result.append("  cmp r8, 0\n");  // Expression results go in r8
      result.append(std::format("  jne .{}{}\n", if_true_label, if_number));
      if (else_is_present)
        result.append(std::format("  jmp .{}{}\n", if_false_label, if_number));
      else
        result.append(std::format("  jmp .{}{}\n", if_end_label, if_number));

      result.append("\n");
      result.append(std::format(".{}{}:\n", if_true_label, if_number));
      result.append(process_ast_node(true_statement_node, function_name));
      if (else_is_present) result.append(std::format("  jmp .{}{}\n", if_end_label, if_number));  // Jump past else

      if (else_is_present) {
        ASTNode &false_statement_node = node.children[2];

        result.append("\n");
        result.append(std::format(".{}{}:\n", if_false_label, if_number));
        result.append(process_ast_node(false_statement_node, function_name));
      }

      result.append(std::format(".{}{}:\n", if_end_label, if_number));

      return result;
    }

    /*-----------------*/
    /* While statement */
    /*-----------------*/
    case AST_NODE_STATEMENT_WHILE: {
      std::string result{};

      int while_number{m_functions_info[function_name].m_while_statement_count++};

      ASTNode &expression_node = node.children[0];
      ASTNode &statement_node = node.children[1];

      result.append(std::format(".{}{}:\n", while_label, while_number));
      result.append(process_ast_node(expression_node, function_name));
      result.append("  cmp r8, 0\n");  // Expression results go in r8
      result.append(std::format("  je .{}{}\n", while_end_label, while_number));
      result.append("\n");
      result.append(process_ast_node(statement_node, function_name));
      result.append(std::format("  jmp .{}{}\n", while_label, while_number));
      result.append(std::format(".{}{}:\n", while_end_label, while_number));

      return result;
    }

    /*------------------*/
    /* Return statement */
    /*------------------*/
    case AST_NODE_STATEMENT_RETURN: {
      std::string result{};

      ASTNode &expression_node = node.children[0];

      result.append(process_ast_node(expression_node, function_name));
      result.append("  mov rax, r8\n");  // The expression result ends up in r8. Return register is rax
      result.append(std::format("  jmp .{}\n", function_end_label));

      return result;
    }

    case AST_NODE_STATEMENT_ASSIGNMENT: {
      std::string result{};
      std::string variable_name{node.data.at("name")};

      ASTNode &expression_node = node.children[0];
      result.append(process_ast_node(expression_node, function_name));  // Expression result will be in r8

      std::unordered_map<std::string, LocalVariable> &local_variables =
          m_functions_info.at(function_name).m_local_variables;

      if (local_variables.contains(variable_name)) {
        LocalVariable &variable_info = local_variables.at(variable_name);

        // TODO: Support floats
        if (variable_info.type == "float") abort("Floats not supported yet");

        result.append(std::format("  mov qword [rbp + {}], rbp\n", variable_info.offset));
      } else {  // Otherwise the variable has global scope (or is undeclared)
        if (!m_global_variables.contains(variable_name)) abort("Unrecognised identifier in assignment");

        std::string variable_type{m_global_variables.at(variable_name)};

        // TODO: Support floats
        if (variable_type == "float") abort("Floats not supported yet");

        result.append(std::format("  mov qword [{}{}], rbp\n", global_id_prefix, variable_name));
      }

      return result;
    }

    // TODO: Put an abort here once all cases filled out
    default: {
      return "";
    }
  }
}

void Emitter::check_function_node_matches_info(ASTNode &function_node, FunctionInfo &function_info) {
  if (function_info.m_return_type != function_node.data.at("return type"))
    abort("Redeclaration of function with different return type");

  size_t parameter_count{0};
  for (const ASTNode &child_node : function_node.children) {
    if (child_node.type != AST_NODE_PARAMETER) break;  // The parameter child nodes are at the front
    ++parameter_count;
  }

  if (parameter_count != function_info.m_parameters.size()) {
    std::cout << parameter_count << " " << function_info.m_parameters.size() << "\n";
    std::cout << function_node.data.at("name") << "\n";
    abort("Redeclaration of function with different number of parameters");
  }

  // Loop through the parameters in order and check they match the existing declaration
  for (auto const &[existing_parameter_name, new_parameter_node] :
       std::views::zip(function_info.m_parameters, function_node.children)) {
    const LocalVariable &existing_parameter_info = function_info.m_local_variables[existing_parameter_name];

    if (existing_parameter_name != new_parameter_node.data.at("name") ||
        existing_parameter_info.type != new_parameter_node.data.at("type"))
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
