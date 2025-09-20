#include <format>
#include <iostream>
#include "token.hpp"
#include "parser.hpp"

bool Parser::program() {
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
    if (!expression()) abort("Expected expression in if statement");
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
    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression in while statement");
    if (!statement()) abort("Expected statement after condition in while statement");

    if (m_print_debug) std::cout << "while statement\n";
    return true;
  }

  // Return statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_RETURN)) {
    expression();
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' or expression then ';' after 'return' in return statement");

    if (m_print_debug) std::cout << "return statement\n";
    return true;
  }

  // Assignment statement
  move_cursor_back_to(entry_cursor_pos);
  if (assignment()) {
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after assignment in assignment statement");

    if (m_print_debug) std::cout << "assignment statement\n";
    return true;
  }

  // Function call statement
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER)) {
    // TODO: Check this is okay if another valid grammar rule starts with an identifier
    if (!token(TOKEN_LPAREN)) abort("Expected '(' after function name in statement");
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

    // TODO: Check if this should just return false
    if (!token(TOKEN_ASSIGN)) abort("Expected '=' in assignment");
    if (!expression()) abort("Expected expression after '=' in assignment");

    if (m_print_debug) std::cout << "assignment\n";
    return true;
  }

  move_cursor_back_to(entry_cursor_pos);
  return false;
}

bool Parser::expression() {
  int entry_cursor_pos{m_cursor_pos};

  // Literal (this comes first as otherwise this function always calls itself)
  if (token(TOKEN_FLOAT_LITERAL) || token(TOKEN_INT_LITERAL) || token(TOKEN_STRING_LITERAL)) {
    if (m_print_debug) std::cout << "literal expression\n";
    return true;
  }

  // Variable, function call or array element
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER)) {
    // Function call
    if (token(TOKEN_LPAREN)) {
      if (expression()) {
        while (token(TOKEN_COMMA)) {
          if (!expression()) abort("Expected expression after ',' in function call");
        }
      }

      if (!token(TOKEN_RPAREN)) abort("Expected ')' at end of function call");

      if (m_print_debug) std::cout << "function call expression\n";
      return true;
    }

    // Array element
    if (token(TOKEN_LBRACKET)) {
      if (!expression()) abort("Expected expression after '['");
      if (!token(TOKEN_RBRACKET)) abort("Expected ']' in expression");

      if (m_print_debug) std::cout << "array element expression\n";
      return true;
    }

    // Variable
    if (m_print_debug) std::cout << "variable expression\n";
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

  // Operation on two expressions
  move_cursor_back_to(entry_cursor_pos);
  if (expression() && (binary_operator() || relational_operator() || logical_operator())) {
    if (!expression()) abort("Expected expression after operator");

    if (m_print_debug) std::cout << "operator expression\n";
    return true;
  }

  // Parenthesised expression
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_LPAREN)) {
    if (!expression()) abort("Expected expression after '('");
    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression");

    if (m_print_debug) std::cout << "parenthesised expression\n";
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
    next_token();
    if (m_print_debug) std::cout << "token (type " << token_type << ")\n";
    return true;
  } else {  // Token does not match
    return false;
  }
}

void Parser::abort(std::string_view message) {
  std::cout << "Compilation aborted: parser error\n-> " << message << "\n";
  std::exit(EXIT_FAILURE);
}

Parser::Parser(Lexer &lexer, bool print_debug)
    : m_lexer{lexer}, m_tokens{}, m_cursor_pos{0}, m_print_debug{print_debug} {
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

  // TODO: Remove this
#if 0
  std::cout << "Cursor moved from " << m_cursor_pos << " to " << idx << " (type = " << m_tokens[idx].get_type()
            << ")\n";
#endif
  m_cursor_pos = idx;
}

void Parser::parse() {
  bool result = program();
  std::cout << "Program is valid: " << result << "\n";
}
