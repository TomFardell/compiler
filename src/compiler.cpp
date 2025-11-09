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
  std::string in_file_name{};
  std::string out_file_name{"a.asm"};
  bool verbose{false};

  for (int i{1}; i < argc; ++i) {
    std::string str_arg{argv[i]};

    if (str_arg == "-o") {
      out_file_name = argv[++i];
    } else if (str_arg == "-v") {
      verbose = true;
    } else if (str_arg[0] == '-') {
      std::cerr << "Compilation aborted\n-> Unknown option type '" << str_arg << "'\n";
      exit(EXIT_FAILURE);
    } else {
      in_file_name = str_arg;
    }
  }

  if (in_file_name == "") {
    std::cerr << "Compilation aborted\n-> Input file name not specified\n";
    exit(EXIT_FAILURE);
  }

  std::string source_string{read_file(in_file_name)};  // Should exist for the lifetime of the lexer and parser

  Lexer lexer{source_string};
  Emitter emitter{out_file_name};
  Parser parser{lexer, emitter, verbose};

  parser.parse();

  return 0;
}
