#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "emitter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

std::string read_file(const std::string file_path) {
  std::ifstream source_stream{file_path};

  std::stringstream source_buffer;
  source_buffer << source_stream.rdbuf();

  return source_buffer.str();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Run the program with a single argument: the file to be compiled, i.e. ./compiler test.c\n";
    std::exit(EXIT_FAILURE);
  }

  std::string in_file_name{argv[1]};

  std::string source_string{read_file(in_file_name)};  // Should exist for the lifetime of the lexer and parser

  Lexer lexer{source_string};
  Emitter emitter{"a.asm"};  // TODO: Add -o argument to change this
  Parser parser{lexer, emitter, true};

  parser.parse();

  return 0;
}
