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
  m_stack_offset += 8;  // Both int and float are 8 bytes in this language
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
      result.append("extern scanf\n");
      result.append("\n");
      result.append("section .data\n");
      result.append("  read_int_fmt: db \"%lld\", 0x0\n");        // Format for scanf to read in an integer
      result.append("  read_float_fmt: db \"%lf\", 0x0\n");       // Format for scanf to read in a float
      result.append("  write_int_fmt: db \"%lld\", 0xA, 0x0\n");  // Format to print integers using printf
      result.append("  write_flt_fmt: db \"%lf\", 0xA, 0x0\n");   // Format to print floats using printf

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

      for (const auto &[function_name, function_info] : m_functions_info) {
        if (function_info.m_is_called && !function_info.m_is_defined) {
          abort(std::format("Call to function '{}' with no existing definition", function_name));
        }
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
      result.append(std::format("  {}{}: resb 8\n", global_id_prefix, variable_name));
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
        function_info.m_is_defined = true;
      } else {
        FunctionInfo &function_info = m_functions_info[function_name];  // Zero initialise function info

        function_info.m_return_type = node.data.at("return type");
        for (const ASTNode &child_node : node.children) {
          if (child_node.type != AST_NODE_PARAMETER) break;
          function_info.add_parameter(child_node.data.at("name"), child_node.data.at("type"));
        }

        function_info.m_is_defined = true;
      }

      FunctionInfo &function_info = m_functions_info.at(function_name);

      result.append(std::format("{}:\n", function_name));
      result.append("  push rbp\n");
      result.append("  mov rbp, rsp\n");

      size_t num_parameters{function_info.m_parameters.size()};
      size_t num_stack_parameters{
          num_parameters > parameter_registers.size() ? num_parameters - parameter_registers.size() : 0};

      // The first parameters will be passed in registers
      for (size_t i = 0; i < std::min(num_parameters, parameter_registers.size()); ++i) {
        result.append(std::format("  push {}\n", parameter_registers[i]));
      }

      // Any remaining parameters were passed on the stack
      for (size_t i = 0; i < num_stack_parameters; ++i) {
        result.append(process_ast_node(node.children[i], function_name));
        // The "+ 1" is because the return address is pushed to the stack when the function is called
        result.append(std::format("  push qword [rbp + {}]\n", 8 * (num_stack_parameters + 1 - i)));
      }

      std::string body{};

      for (ASTNode &child_node : node.children) {
        body.append(process_ast_node(child_node, function_name));
      }

      // Increment the stack pointer through any variables declared in the loop above
      if (function_info.m_local_variables.size() > num_parameters) {
        result.append(
            std::format("  sub rsp, {}\n", 8 * (function_info.m_local_variables.size() - num_parameters)));
      }
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
      result.append(std::format("  cmp {}, 0\n", expression_register));
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
      result.append(std::format("  cmp {}, 0\n", expression_register));
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
      result.append(std::format("  mov rax, {}\n", expression_register));  // Return register is rax
      result.append(std::format("  jmp .{}\n", function_end_label));

      return result;
    }

    /*----------------*/
    /* Read statement */
    /*----------------*/
    case AST_NODE_STATEMENT_READ: {
      std::string result{};
      std::string variable_name{node.data.at("name")};

      std::unordered_map<std::string, LocalVariable> &local_variables =
          m_functions_info.at(function_name).m_local_variables;

      if (local_variables.contains(variable_name)) {
        LocalVariable &variable_info = local_variables.at(variable_name);

        // TODO: Support floats
        if (variable_info.type == "float") abort("Floats not supported yet");

        result.append(std::format("  mov {}, read_int_fmt\n", parameter_registers[0]));
        result.append(std::format("  mov {}, [rbp + {}]\n", parameter_registers[1], variable_info.offset));
      } else {  // Variable has global scope (or is undefined)
        if (!m_global_variables.contains(variable_name)) abort("Unrecognised identifier in write statement");

        std::string variable_type{m_global_variables.at(variable_name)};

        // TODO: Support floats
        if (variable_type == "float") abort("Floats not supported yet");

        result.append(std::format("  mov {}, read_int_fmt\n", parameter_registers[0]));
        result.append(std::format("  mov {}, {}{}\n", parameter_registers[0], global_id_prefix, variable_name));
      }

      result.append("  call scanf\n");
      result.append("\n");

      return result;
    }

    /*-----------------*/
    /* Write statement */
    /*-----------------*/
    case AST_NODE_STATEMENT_WRITE: {
      std::string result{};
      ASTNode &write_node = node.children[0];

      if (write_node.type == AST_NODE_STRING_LITERAL) {
        result.append(std::format("  mov {}, {}{}\n", parameter_registers[0], string_literal_id,
                                  write_node.data.at("number")));
      } else {  // Otherwise is an expression
        result.append(process_ast_node(write_node, function_name));

        // TODO: Handle float case here
        result.append(std::format("  mov {}, write_int_fmt\n", parameter_registers[0]));
        result.append(std::format("  mov {}, r8\n", parameter_registers[1]));
      }

      result.append("  call printf\n");
      result.append("\n");

      return result;
    }

    /*---------------------------------------*/
    /* Function call statement or expression */
    /*---------------------------------------*/
    case AST_NODE_STATEMENT_FUNCTION_CALL:
    case AST_NODE_EXPRESSION_FUNCTION_CALL: {
      std::string result{};

      std::string called_function_name{node.data.at("name")};
      if (!m_functions_info.contains(called_function_name)) abort("Call to undeclared function in statement");

      FunctionInfo &function_info = m_functions_info.at(called_function_name);
      function_info.m_is_called = true;

      size_t num_arguments_given{node.children.size()};
      size_t num_arguments_expected{function_info.m_parameters.size()};

      if (num_arguments_given != num_arguments_expected)
        abort("Incorrect number of arguments given to function call in statement");

      // The first arguments go in registers
      for (size_t i = 0; i < std::min(num_arguments_given, parameter_registers.size()); ++i) {
        result.append(process_ast_node(node.children[i], function_name));
        result.append(std::format("  mov {}, {}\n", parameter_registers[i], expression_register));
      }

      // Any remaining arguments go on the stack
      for (size_t i = parameter_registers.size(); i < num_arguments_given; ++i) {
        result.append(process_ast_node(node.children[i], function_name));
        result.append(std::format("  push {}\n", expression_register));
      }

      result.append(std::format("  call {}\n", called_function_name));

      // If arguments were pushed to the stack, move the stack pointer back over the arguments
      if (parameter_registers.size() < num_arguments_given) {
        size_t stack_increment{8 * (num_arguments_given - parameter_registers.size())};
        result.append(std::format("  add rbp, {}\n", stack_increment));
        result.append("\n");
      }

      // If the function call is an expression, put the returned value in the expression register
      if (node.type == AST_NODE_EXPRESSION_FUNCTION_CALL) {
        result.append(std::format("  mov {}, rax\n", expression_register));
      } else {
        result.append("\n");
      }

      return result;
    }

    /*----------------------*/
    /* Assignment statement */
    /*----------------------*/
    case AST_NODE_STATEMENT_ASSIGNMENT: {
      std::string result{};
      std::string variable_name{node.data.at("name")};

      ASTNode &expression_node = node.children[0];
      result.append(process_ast_node(expression_node, function_name));

      std::unordered_map<std::string, LocalVariable> &local_variables =
          m_functions_info.at(function_name).m_local_variables;

      if (local_variables.contains(variable_name)) {
        LocalVariable &variable_info = local_variables.at(variable_name);

        // TODO: Support floats
        if (variable_info.type == "float") abort("Floats not supported yet");

        result.append(std::format("  mov qword [rbp + {}], {}\n", variable_info.offset, expression_register));
      } else {  // Otherwise the variable has global scope (or is undeclared)
        if (!m_global_variables.contains(variable_name)) abort("Unrecognised identifier in assignment statement");

        std::string variable_type{m_global_variables.at(variable_name)};

        // TODO: Support floats
        if (variable_type == "float") abort("Floats not supported yet");

        result.append(
            std::format("  mov qword [{}{}], {}\n", global_id_prefix, variable_name, expression_register));
      }

      result.append("\n");

      return result;
    }

    /*-----------------------*/
    /* Braced statement list */
    /*-----------------------*/
    case AST_NODE_STATEMENT_LIST: {
      std::string result{};

      for (ASTNode &child_node : node.children) {
        result.append(process_ast_node(child_node, function_name));
      }

      return result;
    }

    /*-----------------*/
    /* Empty statement */
    /*-----------------*/
    case AST_NODE_STATEMENT_EMPTY: {
      return "";
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
