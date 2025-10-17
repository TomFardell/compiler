#include "ast.hpp"

#include <iostream>
#include <string>

void ASTNode::print_tree(int indent) {
  std::string prefix{};
  for (int i = 0; i < indent; ++i) {
    prefix += "| ";
  }

  std::cout << prefix << "*---\n";
  std::cout << prefix << "| Type: " << ASTNode::type_names.at(type) << "\n";

  if (data.size() > 0) {
    std::cout << prefix << "| Data: [";
    bool first_print{true};  // Just to prevent a trailing comma in the printed map
    for (auto [key, value] : data) {
      if (first_print)
        first_print = false;
      else
        std::cout << ", ";
      std::cout << "\"" << key << "\": \"" << value << "\"";
    }
    std::cout << "]\n";
  }

  if (children.size() > 0) {
    std::cout << prefix << "| Children:\n";
    for (ASTNode child_node : children) {
      child_node.print_tree(indent + 1);
    }
  }

  std::cout << prefix << "*---\n";
}
