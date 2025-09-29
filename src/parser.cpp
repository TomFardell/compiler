#include <format>
#include <iostream>
#include "emitter.hpp"
#include "parser.hpp"
#include "token.hpp"

bool Parser::program() {
  m_emitter.add_line_to_header(".section .rodata");
  m_emitter.add_line_to_header(R"(.int_format: .string "%d\n")");
  m_emitter.add_line_to_header(R"(.flt_format: .string "%f\n")");
  m_emitter.add_line_to_header(R"(.str_format: .string "%s\n")");
  m_emitter.add_line_to_code(".text");
  m_emitter.add_line_to_code(".globl main");
  m_emitter.add_line_to_code(".type main,@function");

  while (!token(TOKEN_EOF)) {
    if (function()) {
      continue;
    }

    if (declaration()) {
      if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after declaration");

      continue;
    }

    move_cursor_back_to(0);  // Not strictly necessary, but keeps this function consistent with the others
    return false;
  }

  if (m_print_debug) std::cout << "program\n";
  return true;
}

bool Parser::declaration() {
  int entry_cursor_pos{m_cursor_pos};

  // Function declaration
  if ((token(TOKEN_VOID) || type()) && token(TOKEN_IDENTIFIER) && token(TOKEN_LPAREN) && parameter_types() &&
      token(TOKEN_RPAREN)) {
    while (token(TOKEN_COMMA)) {
      if (!(token(TOKEN_IDENTIFIER) && token(TOKEN_LPAREN) && parameter_types() && token(TOKEN_RPAREN)))
        abort("Expected function declaration after ','");
    }

    if (m_print_debug) std::cout << "function declaration\n";
    return true;
  }

  // Variable declaration
  move_cursor_back_to(entry_cursor_pos);
  if (type() && variable_declaration()) {
    while (token(TOKEN_COMMA)) {
      if (!variable_declaration()) abort("Expected variable declaration after ','");
    }

    if (m_print_debug) std::cout << "variable declaration\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::parameter_types() {
  int entry_cursor_pos{m_cursor_pos};

  if (token(TOKEN_VOID)) {
    if (m_print_debug) std::cout << "void parameter\n";
    return true;
  }

  if (type() && token(TOKEN_IDENTIFIER)) {
    array_declaration_suffix();

    while (token(TOKEN_COMMA)) {
      if (type() && token(TOKEN_IDENTIFIER)) {
        array_declaration_suffix();
      } else {
        abort("Expected more parameters after ','");
      }
    }

    if (m_print_debug) std::cout << "typed parameters\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::function() {
  int entry_cursor_pos{m_cursor_pos};

  if ((token(TOKEN_VOID) || type()) && token(TOKEN_IDENTIFIER) && token(TOKEN_LPAREN) && parameter_types() &&
      token(TOKEN_RPAREN) && token(TOKEN_LBRACE)) {
    while (type()) {
      if (!variable_declaration()) abort("Expected variable declaration after type");

      while (token(TOKEN_COMMA)) {
        if (!variable_declaration()) abort("Expected variable declaration after ','");
      }

      if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after declaration");
    }

    while (statement());

    if (!token(TOKEN_RBRACE)) abort("Expected '}' at end of function definition");

    if (m_print_debug) std::cout << "function definition\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::variable_declaration() {
  int entry_cursor_pos{m_cursor_pos};

  if (token(TOKEN_IDENTIFIER)) {
    array_declaration_suffix();

    if (m_print_debug) std::cout << "variable to declare\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::array_declaration_suffix() {
  int entry_cursor_pos{m_cursor_pos};

  if (token(TOKEN_LBRACKET)) {
    if (!token(TOKEN_INT_LITERAL)) abort("Expected int literal after '['");
    if (!token(TOKEN_RBRACKET)) abort("Expected ']' at end of array declaration suffix");

    if (m_print_debug) std::cout << "array declaration suffix\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::type() {
  if (token(TOKEN_FLOAT) || token(TOKEN_INT)) {
    if (m_print_debug) std::cout << "type\n";

    return true;
  }

  return false;
}

bool Parser::statement() {
  int entry_cursor_pos{m_cursor_pos};

  // If statement
  if (token(TOKEN_IF)) {
    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'if' in if statement");
    if (!expression()) abort("Expected expression after '(' in if statement");
    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression in if statement");

    if (!statement()) abort("Expected statement after condition in if statement");
    if (token(TOKEN_ELSE)) {
      if (!statement()) abort("Expected statement after 'else' in if statement");
    }

    if (m_print_debug) std::cout << "if statement\n";
    return true;
  }

  // While statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_WHILE)) {
    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'while' in while statement");
    if (!expression()) abort("Expected expression in while statement");
    if (token(TOKEN_RPAREN)) {
      if (!statement()) abort("Expected statement after condition in while statement");

      if (m_print_debug) std::cout << "while statement\n";
      return true;
    }
  }

  // Return statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_RETURN)) {
    expression();
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' or expression then ';' after 'return' in return statement");

    if (m_print_debug) std::cout << "return statement\n";
    return true;
  }

  // Read statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_READ)) {
    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'read' in read statement");
    if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after '(' in read statement");
    if (!token(TOKEN_RPAREN)) abort("Expected '(' after identifier in read statement");
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after ')' in read statement");

    if (m_print_debug) std::cout << "read statement\n";
    return true;
  }

  // Write statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_WRITE)) {
    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'write' in write statement");
    if (!(token(TOKEN_STRING_LITERAL) || expression()))
      abort("Expected string literal or expression after '(' in write statement");
    if (!token(TOKEN_RPAREN)) abort("Expected '(' after identifier in write statement");
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after ')' in write statement");

    if (m_print_debug) std::cout << "write statement\n";
    return true;
  }

  // Function call statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER) && token(TOKEN_LPAREN)) {
    if (expression()) {
      while (token(TOKEN_COMMA)) {
        if (!expression()) abort("Expected expression after ','");
      }
    }
    if (!token(TOKEN_RPAREN)) abort("Expected ')' at end of function call in statement");
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after function call in statement");

    if (m_print_debug) std::cout << "function call statement\n";
    return true;
  }

  // Assignment statement
  move_cursor_back_to(entry_cursor_pos);
  if (assignment()) {
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after assignment in assignment statement");

    if (m_print_debug) std::cout << "assignment statement\n";
    return true;
  }

  // Braced statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_LBRACE)) {
    if (!statement()) abort("Expected statement after '{'");
    if (!token(TOKEN_RBRACE)) abort("Expected '}' after braced statement");

    if (m_print_debug) std::cout << "braced statement\n";
    return true;
  }

  // Lone semicolon
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_SEMICOLON)) {
    if (m_print_debug) std::cout << "lone semicolon statement\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::assignment() {
  int entry_cursor_pos{m_cursor_pos};

  if (token(TOKEN_IDENTIFIER)) {
    if (token(TOKEN_LBRACKET)) {
      if (!expression()) abort("Expected expression after '[' in assignment");
      if (!token(TOKEN_RBRACKET)) abort("Expected ']' after expression in assignment");
    }

    // Don't abort as this would be erroneous in the case of a lone function call
    if (!token(TOKEN_ASSIGN)) {
      move_cursor_back_to(entry_cursor_pos);
      return false;
    }

    if (!expression()) abort("Expected expression after '=' in assignment");

    if (m_print_debug) std::cout << "assignment\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::expression() {
  int entry_cursor_pos{m_cursor_pos};

  // Parenthesised expression
  if (token(TOKEN_LPAREN)) {
    if (!expression()) abort("Expected expression after '('");
    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression");

    // Greedily search for an operator
    if (binary_operator() || relational_operator() || logical_operator()) {
      if (!expression()) abort("Expected expression after operator");

      if (m_print_debug) std::cout << "operator expression\n";
      return true;
    }

    if (m_print_debug) std::cout << "parenthesised expression\n";
    return true;
  }

  // Negative expression
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_MINUS)) {
    if (!expression()) abort("Expected expression after '-");

    if (m_print_debug) std::cout << "negative expression\n";
    return true;
  }

  // Negated expression
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_NOT)) {
    if (!expression()) abort("Expected expression after '!'");

    if (m_print_debug) std::cout << "negated expression\n";
    return true;
  }

  // Expressions beginning with a literal
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_FLOAT_LITERAL) || token(TOKEN_INT_LITERAL) || token(TOKEN_STRING_LITERAL)) {
    // Greedily search for an operator
    if (binary_operator() || relational_operator() || logical_operator()) {
      if (!expression()) abort("Expected expression after operator");

      if (m_print_debug) std::cout << "operator expression\n";
      return true;
    }

    // Only accept lone literal expression if no operator was found afterwards
    if (m_print_debug) std::cout << "literal expression\n";
    return true;
  }

  // Expressions beginning with an identifier
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER)) {
    // Function call identifier
    if (token(TOKEN_LPAREN)) {
      if (expression()) {
        while (token(TOKEN_COMMA)) {
          if (!expression()) abort("Expected expression after ',' in function call");
        }
      }

      if (!token(TOKEN_RPAREN)) abort("Expected ')' at end of function call");

      // Greedily search for an operator
      if (binary_operator() || relational_operator() || logical_operator()) {
        if (!expression()) abort("Expected expression after operator");

        if (m_print_debug) std::cout << "operator expression\n";
        return true;
      }
    }

    // Array element identifier
    if (token(TOKEN_LBRACKET)) {
      if (!expression()) abort("Expected expression after '['");
      if (!token(TOKEN_RBRACKET)) abort("Expected ']' in expression");

      // Greedily search for an operator
      if (binary_operator() || relational_operator() || logical_operator()) {
        if (!expression()) abort("Expected expression after operator");

        if (m_print_debug) std::cout << "operator expression\n";
        return true;
      }
    }

    // Greedily search for an operator
    if (binary_operator() || relational_operator() || logical_operator()) {
      if (!expression()) abort("Expected expression after operator");

      if (m_print_debug) std::cout << "operator expression\n";
      return true;
    }

    if (m_print_debug) std::cout << "variable expression\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::binary_operator() {
  if (token(TOKEN_PLUS) || token(TOKEN_MINUS) || token(TOKEN_MULTIPLY) || token(TOKEN_DIVIDE)) {
    if (m_print_debug) std::cout << "binary operator\n";
    return true;
  }

  return false;
}

bool Parser::relational_operator() {
  if (token(TOKEN_EQ) || token(TOKEN_NEQ) || token(TOKEN_LT) || token(TOKEN_LE) || token(TOKEN_GT) ||
      token(TOKEN_GE)) {
    if (m_print_debug) std::cout << "relational operator\n";
    return true;
  }

  return false;
}

bool Parser::logical_operator() {
  if (token(TOKEN_AND) || token(TOKEN_OR)) {
    if (m_print_debug) std::cout << "logical operator\n";
    return true;
  }

  return false;
}

bool Parser::token(TokenType token_type) {
  if (m_tokens[m_cursor_pos].get_type() == token_type) {  // Token matches
    if (m_print_debug)
      std::cout << "token " << Token::type_names.at(token_type) << ": '" << m_tokens[m_cursor_pos].get_text()
                << "'\n";
    next_token();

    return true;
  } else {  // Token does not match
    return false;
  }
}

void Parser::abort(std::string_view message) {
  std::cout << "Compilation aborted: parser error\n-> " << message << "\n";
  std::exit(EXIT_FAILURE);
}

Parser::Parser(Lexer& lexer, Emitter& emitter, bool print_debug)
    : m_lexer{lexer}, m_emitter{emitter}, m_tokens{}, m_cursor_pos{0}, m_print_debug{print_debug} {
  m_tokens.emplace_back(m_lexer.get_token());
};

void Parser::next_token() {
  ++m_cursor_pos;

  // If the new cursor position goes past the stored tokens, read another
  if (m_cursor_pos >= static_cast<int>(m_tokens.size())) {
    m_tokens.emplace_back(m_lexer.get_token());
  }
}

void Parser::move_cursor_back_to(int idx) {
  if (idx > m_cursor_pos) {  // If the index is ahead abort
    abort(std::format("Cannot move cursor forwards from %d to %d", m_cursor_pos, idx));
  }

  m_cursor_pos = idx;
}

void Parser::parse() {
  bool result = program();
  if (result) m_emitter.write_file();
  std::cout << "Program is valid: " << result << "\n";
}
