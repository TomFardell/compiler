#ifndef EMITTER_H
#define EMITTER_H

#include <fstream>
#include <string>

class Emitter {
 private:
  const std::string m_out_path;  // File path of the compiled code
  std::string m_header;          // Header to go at the top of the emitted file
  std::string m_code;            // Main body of code in the emitted file

 public:
  // Constructor taking out file path
  Emitter(const std::string out_path) : m_out_path{out_path}, m_header{}, m_code{} {};
  // Add the given line to the stored code with a newline added
  void add_line_to_code(const std::string line) { m_code += line + "\n"; }
  // Add the given line to the header with a newline added
  void add_line_to_header(const std::string line) { m_header += line + "\n"; }
  // Write the stored header and code to the out file
  void write_file() {
    std::ofstream file{m_out_path};
    file << m_header << m_code;
    file.close();
  }
};

#endif
