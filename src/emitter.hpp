#ifndef EMITTER_H
#define EMITTER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hpp"

struct LocalVariable {
  std::string type;  // Type of the local variable
  int offset;        // Offset of the local variable (from rbp)
};

class FunctionInfo {
 public:
  std::string m_return_type;                                         // Return type of the function
  std::vector<std::string> m_parameters;                             // Names of parameters in order
  std::unordered_map<std::string, LocalVariable> m_local_variables;  // Types and offsets of local variables

  int m_stack_offset;  // Offset of the next local variable to be added
  bool m_is_defined;   // Whether a definition of the function exists

  FunctionInfo() : m_return_type{}, m_parameters{}, m_local_variables{}, m_stack_offset{0}, m_is_defined{false} {};

  // Add a local variable to the store while incrementing the offset
  void add_local_variable(std::string name, std::string type);
  // Add a parameter to the store while incrementing the offset
  void add_parameter(std::string name, std::string type);
};

class Emitter {
 private:
  const std::string m_out_path;  // File path of the compiled code

  std::unordered_map<std::string, FunctionInfo> m_functions_info;   // Lookup for info on each declared function
  std::unordered_map<std::string, std::string> m_global_variables;  // Lookup for types of global variables

  // Given an abstract syntax tree node, get the assembly code associated with that node. Calling this with a
  // program node will return the entire program in assembly
  std::string process_ast_node(ASTNode &node);
  // Check whether a redeclaration of a given function matches the exisiting info, aborting if not
  void check_function_node_matches_info(ASTNode &function_node, FunctionInfo &function_info);

  // Stop the compilation due to an emission error
  void abort(std::string_view);

 public:
  std::vector<std::string> m_string_literals;  // Vector containing all string literals appearing in the program

  const std::string m_string_literal_identifier{"str_lit"};  // Name of string literals in the assembly

  // Constructor taking out file path
  Emitter(const std::string out_path) : m_out_path{out_path}, m_functions_info{}, m_global_variables{} {};

  // Emit the program with the given root node to the outfile
  void emit_program(ASTNode &program_node);
};

#endif
