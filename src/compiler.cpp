#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Run the program with a single argument: the file to be compiled, i.e. ./compiler test.c\n";
    std::exit(EXIT_FAILURE);
  }

  // Read the file into a stringstream
  std::ifstream source_stream{argv[1]};
  std::stringstream source_buffer;
  source_buffer << source_stream.rdbuf();

  Lexer lexer{source_buffer.str()};
  Parser parser{lexer, true};

  parser.parse();

  return 0;
}
