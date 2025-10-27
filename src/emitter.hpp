#ifndef EMITTER_H
#define EMITTER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ast.hpp"

struct Parameter {
  std::string name;  // Name of the parameter
  std::string type;  // Type of the parameter
};

struct FunctionInfo {
  std::string return_type;                                     // Return type of the function
  std::vector<Parameter> parameters;                           // Names and types of parameters for the function
  std::unordered_map<std::string, int> variable_byte_offsets;  // Number of bytes each local variable is offset by
  int variable_current_offset;                                 // Offset of the next local variable to be added
};

class Emitter {
 private:
  const std::string m_out_path;  // File path of the compiled code

  std::unordered_map<std::string, FunctionInfo> m_functions_info;  // Lookup for info on each declared function
  std::unordered_set<std::string> m_global_variables;  // Global variables that have been declared so far

  // Given an abstract syntax tree node, get the assembly code associated with that node. Calling this with a
  // program node will return the entire program in assembly
  std::string process_ast_node(ASTNode &node);

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
