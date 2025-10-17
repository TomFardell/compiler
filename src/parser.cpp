#include "parser.hpp"

#include <format>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "emitter.hpp"
#include "token.hpp"

ASTNode Parser::program() {
  ASTNode program_node{AST_NODE_PROGRAM, {}, {}};

  while (!token(TOKEN_EOF)) {
    ASTNode function_definition_node{function()};
    if (function_definition_node.type != AST_NODE_NULL) {
      program_node.children.push_back(function_definition_node);
      continue;
    }

    std::vector<ASTNode> declaration_nodes{declaration()};
    if (declaration_nodes.size() != 0) {
      if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after declaration");

      program_node.children.insert(program_node.children.end(), declaration_nodes.begin(),
                                   declaration_nodes.end());
      continue;
    }

    move_cursor_back_to(0);  // Not strictly necessary, but keeps this function consistent with the others
    return {AST_NODE_NULL, {}, {}};
  }

  if (m_print_debug) std::cout << "program\n";
  return program_node;
}

std::vector<ASTNode> Parser::declaration() {
  int entry_cursor_pos{m_cursor_pos};

  /*----------------------*/
  /* Function declaration */
  /*----------------------*/
  std::vector<ASTNode> function_declaration_nodes{{AST_NODE_FUNCTION_DECLARATION, {}, {}}};
  do {  // I will be using do-while-false to allow me to use break when the current sequence does not match
    if (token(TOKEN_VOID)) {
      function_declaration_nodes[0].data["return type"] = "void";
    } else {
      std::string type_name{type()};
      if (type_name != "") {
        function_declaration_nodes[0].data["return type"] = type_name;
      } else {
        break;
      }
    }

    if (!token(TOKEN_IDENTIFIER)) break;
    function_declaration_nodes[0].data["name"] = m_tokens[m_cursor_pos - 1].get_text();

    if (!token(TOKEN_LPAREN)) break;

    // I will use empty scopes like this one when I want certain variables (here parameter_type_nodes) to expire
    // once the scope ends. Here, this is so I don't need to name the variable something different when I do the
    // same thing in the while loop after a comma. Note this would technically work in this case, but I get a
    // language server warning about shadowing, which I would like to avoid. The code in the while loop is slightly
    // different (in that it aborts more) so it does make sense to repeat the similar code in this manner
    {
      std::vector<ASTNode> parameter_type_nodes{parameter_types()};
      if (parameter_type_nodes.size() > 0) {
        function_declaration_nodes[0].children.insert(function_declaration_nodes[0].children.end(),
                                                      parameter_type_nodes.begin(), parameter_type_nodes.end());
      } else {
        // Can only abort here because we already looked for function definitions first in program()
        abort("Expected parameters after '(' in function declaration");
      }
    }

    if (!token(TOKEN_RPAREN)) abort("Expected ')' after parameters in function declaration");

    while (token(TOKEN_COMMA)) {
      function_declaration_nodes.emplace_back(
          AST_NODE_FUNCTION_DECLARATION, std::unordered_map<std::string, std::string>{}, std::vector<ASTNode>{});

      if (!token(TOKEN_IDENTIFIER)) abort("Expected another identifier after ',' after function declaration");
      if (!token(TOKEN_LPAREN)) abort("Expected '(' after identifier in function declaration after ','");

      std::vector<ASTNode> parameter_type_nodes{parameter_types()};
      if (parameter_type_nodes.size() > 0) {
        function_declaration_nodes.back().children.insert(function_declaration_nodes.back().children.end(),
                                                          parameter_type_nodes.begin(),
                                                          parameter_type_nodes.end());
      } else {
        abort("Expected parameters after '(' in function declaration after ','");
      }

      if (!token(TOKEN_RPAREN)) abort("Expected ')' after parameters in function declaration after ','");
    }

    if (m_print_debug) std::cout << "function declaration\n";
    return function_declaration_nodes;
  } while (false);

  /*----------------------*/
  /* Variable declaration */
  /*----------------------*/
  move_cursor_back_to(entry_cursor_pos);
  std::vector<ASTNode> variable_declaration_nodes{};
  do {
    std::string type_name{type()};
    if (type_name == "") break;

    if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after type in variable declaration");
    variable_declaration_nodes.emplace_back(
        AST_NODE_VARIABLE_DECLARATION,
        std::unordered_map<std::string, std::string>{{"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}},
                                                     {"type", type_name}},
        std::vector<ASTNode>{});

    while (token(TOKEN_COMMA)) {
      if (!token(TOKEN_IDENTIFIER)) abort("Expected variable declaration after ','");
      variable_declaration_nodes.emplace_back(
          AST_NODE_VARIABLE_DECLARATION,
          std::unordered_map<std::string, std::string>{
              {"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}}, {"type", type_name}},
          std::vector<ASTNode>{});
    }

    if (m_print_debug) std::cout << "variable declaration\n";
    return variable_declaration_nodes;

  } while (false);

  move_cursor_back_to(entry_cursor_pos);
  return {};
}

std::vector<ASTNode> Parser::parameter_types() {
  int entry_cursor_pos{m_cursor_pos};

  /*----------------*/
  /* Void parameter */
  /*----------------*/
  if (token(TOKEN_VOID)) {
    if (m_print_debug) std::cout << "void parameter\n";
    return {{AST_NODE_VOID_PARAMETERS, {}, {}}};
  }

  /*----------------*/
  /* Parameter list */
  /*----------------*/
  std::vector<ASTNode> parameter_nodes{};
  do {
    {
      std::string type_name{type()};
      if (type_name == "") break;
      if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after type in parameter list");

      parameter_nodes.emplace_back(
          AST_NODE_PARAMETER,
          std::unordered_map<std::string, std::string>{
              {"type", type_name}, {"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}}},
          std::vector<ASTNode>{});
    }

    while (token(TOKEN_COMMA)) {
      std::string type_name{type()};
      if (type_name == "") abort("Expected type name after ',' in parameter list");
      if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after type in parameter list");

      parameter_nodes.emplace_back(
          AST_NODE_PARAMETER,
          std::unordered_map<std::string, std::string>{
              {"type", type_name}, {"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}}},
          std::vector<ASTNode>{});
    }

    if (m_print_debug) std::cout << "parameter list\n";
    return parameter_nodes;

  } while (false);

  move_cursor_back_to(entry_cursor_pos);
  return {};
}

ASTNode Parser::function() {
  int entry_cursor_pos{m_cursor_pos};

  /*---------------------*/
  /* Function definition */
  /*---------------------*/
  ASTNode function_node{AST_NODE_FUNCTION_DEFINITION, {}, {}};
  do {
    if (token(TOKEN_VOID)) {
      function_node.data["return type"] = "void";
    } else {
      std::string type_name{type()};
      if (type_name != "") {
        function_node.data["return type"] = type_name;
      } else {
        break;
      }
    }

    if (!token(TOKEN_IDENTIFIER)) break;
    function_node.data["name"] = m_tokens[m_cursor_pos - 1].get_text();

    if (!token(TOKEN_LPAREN)) break;

    std::vector<ASTNode> parameter_type_nodes{parameter_types()};
    if (parameter_type_nodes.size() == 0)
      abort("Expected parameters after '(' in function declaration/definition");
    function_node.children.insert(function_node.children.end(), parameter_type_nodes.begin(),
                                  parameter_type_nodes.end());

    if (!token(TOKEN_RPAREN)) abort("Expected ')' after parameters in function declaration/definition");
    if (!token(TOKEN_LBRACE)) break;

    // Loop through variable declarations
    std::vector<ASTNode> variable_declaration_nodes{};
    for (std::string type_name{type()}; type_name != ""; type_name = type()) {
      if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after type");
      variable_declaration_nodes.emplace_back(
          AST_NODE_VARIABLE_DECLARATION,
          std::unordered_map<std::string, std::string>{
              {"type", type_name}, {"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}}},
          std::vector<ASTNode>{});

      while (token(TOKEN_COMMA)) {
        if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after ','");
        variable_declaration_nodes.emplace_back(
            AST_NODE_VARIABLE_DECLARATION,
            std::unordered_map<std::string, std::string>{
                {"type", type_name}, {"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}}},
            std::vector<ASTNode>{});
      }

      if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after declaration");
    }
    function_node.children.insert(function_node.children.end(), variable_declaration_nodes.begin(),
                                  variable_declaration_nodes.end());

    // Loop through statements
    std::vector<ASTNode> statement_nodes{};
    for (ASTNode statement_node{statement()}; statement_node.type != AST_NODE_NULL; statement_node = statement()) {
      statement_nodes.push_back(statement_node);
    }
    function_node.children.insert(function_node.children.begin(), statement_nodes.begin(), statement_nodes.end());

    if (!token(TOKEN_RBRACE)) abort("Expected '}' at end of function declaration");

    if (m_print_debug) std::cout << "function definition\n";
    return function_node;
  } while (false);

  move_cursor_back_to(entry_cursor_pos);
  return {AST_NODE_NULL, {}, {}};
}

std::string Parser::type() {
  /*------------*/
  /* Float type */
  /*------------*/
  if (token(TOKEN_FLOAT)) {
    if (m_print_debug) std::cout << "float type\n";
    return "float";
  }

  /*--------------*/
  /* Integer type */
  /*--------------*/
  if (token(TOKEN_INT)) {
    if (m_print_debug) std::cout << "int type\n";
    return "int";
  }

  return "";
}

ASTNode Parser::statement() {
  int entry_cursor_pos{m_cursor_pos};

  /*--------------*/
  /* If statement */
  /*--------------*/
  if (token(TOKEN_IF)) {
    ASTNode if_statement_node{AST_NODE_STATEMENT_IF, {}, {}};

    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'if' in if statement");

    if_statement_node.children.push_back(expression());
    if (if_statement_node.children.back().type == AST_NODE_NULL)
      abort("Expected expression after '(' in if statement");

    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression in if statement");

    if_statement_node.children.push_back(statement());
    if (if_statement_node.children.back().type == AST_NODE_NULL)
      abort("Expected statement after condition in if statement");

    if (token(TOKEN_ELSE)) {
      if_statement_node.children.push_back(statement());
      if (if_statement_node.children.back().type == AST_NODE_NULL)
        abort("Expected statement after 'else' in if statement");
    }

    if (m_print_debug) std::cout << "if statement\n";
    return if_statement_node;
  }

  /*-----------------*/
  /* While statement */
  /*-----------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_WHILE)) {
    ASTNode while_statement_node{AST_NODE_STATEMENT_WHILE, {}, {}};

    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'while' in while statement");

    while_statement_node.children.push_back(expression());
    if (while_statement_node.children.back().type == AST_NODE_NULL)
      abort("Expected expression in while statement");

    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression in while statement");

    while_statement_node.children.push_back(statement());
    if (while_statement_node.children.back().type == AST_NODE_NULL)
      abort("Expected statement after condition in while statement");

    if (m_print_debug) std::cout << "while statement\n";
    return while_statement_node;
  }

  /*------------------*/
  /* Return statement */
  /*------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_RETURN)) {
    ASTNode return_statement_node{AST_NODE_STATEMENT_RETURN, {}, {}};

    ASTNode expression_node{expression()};
    if (expression_node.type != AST_NODE_NULL) return_statement_node.children.push_back(expression_node);

    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' or expression then ';' after 'return' in return statement");

    if (m_print_debug) std::cout << "return statement\n";
    return return_statement_node;
  }

  /*----------------*/
  /* Read statement */
  /*----------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_READ)) {
    ASTNode read_statement_node{AST_NODE_STATEMENT_READ, {}, {}};

    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'read' in read statement");

    if (!token(TOKEN_IDENTIFIER)) abort("Expected identifier after '(' in read statement");
    read_statement_node.data["name"] = m_tokens[m_cursor_pos - 1].get_text();

    if (!token(TOKEN_RPAREN)) abort("Expected '(' after identifier in read statement");

    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after ')' in read statement");

    if (m_print_debug) std::cout << "read statement\n";
    return read_statement_node;
  }

  /*-----------------*/
  /* Write statement */
  /*-----------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_WRITE)) {
    ASTNode write_statement_node{AST_NODE_STATEMENT_WRITE, {}, {}};

    if (!token(TOKEN_LPAREN)) abort("Expected '(' after 'write' in write statement");

    ASTNode expression_node{expression()};
    if (expression_node.type != AST_NODE_NULL) {
      write_statement_node.children.push_back(expression_node);
    } else if (token(TOKEN_STRING_LITERAL)) {
      write_statement_node.children.emplace_back(
          AST_NODE_STRING_LITERAL, std::unordered_map<std::string, std::string>{}, std::vector<ASTNode>{});
    } else {
      abort("Expected string literal or expression after '(' in write statement");
    }

    if (!token(TOKEN_RPAREN)) abort("Expected '(' after identifier in write statement");

    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after ')' in write statement");

    if (m_print_debug) std::cout << "write statement\n";
    return write_statement_node;
  }

  /*-------------------------*/
  /* Function call statement */
  /*-------------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER) && token(TOKEN_LPAREN)) {
    ASTNode function_call_node{AST_NODE_STATEMENT_FUNCTION_CALL, {}, {}};
    function_call_node.data["name"] = m_tokens[m_cursor_pos - 2].get_text();

    std::vector<ASTNode> argument_expression_nodes{};
    argument_expression_nodes.push_back(expression());
    if (argument_expression_nodes.back().type != AST_NODE_NULL) {
      while (token(TOKEN_COMMA)) {
        argument_expression_nodes.push_back(expression());
        if (argument_expression_nodes.back().type == AST_NODE_NULL) abort("Expected expression after ','");
      }
      function_call_node.children.insert(function_call_node.children.begin(), argument_expression_nodes.begin(),
                                         argument_expression_nodes.end());
    }

    if (!token(TOKEN_RPAREN)) abort("Expected ')' at end of function call in statement");
    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after function call in statement");

    if (m_print_debug) std::cout << "function call statement\n";
    return function_call_node;
  }

  /*----------------------*/
  /* Assignment statement */
  /*----------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER) && token(TOKEN_ASSIGN)) {
    ASTNode assignment_node{AST_NODE_STATEMENT_ASSIGNMENT, {}, {}};

    assignment_node.children.push_back(expression());
    if (assignment_node.children.back().type == AST_NODE_NULL)
      abort("Expected expression after '=' in assignment statement");

    if (!token(TOKEN_SEMICOLON)) abort("Expected ';' after assignment statement");

    if (m_print_debug) std::cout << "assignment\n";
    return assignment_node;
  }

  /*-------------------*/
  /* Braced statements */
  /*-------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_LBRACE)) {
    ASTNode statement_nodes{AST_NODE_STATEMENT_LIST, {}, {}};
    while (!token(TOKEN_RBRACE)) {
      statement_nodes.children.push_back(statement());
      if (statement_nodes.children.back().type == AST_NODE_NULL) abort("Invalid statement in braced scope");
    }

    if (m_print_debug) std::cout << "braced statement\n";
    return statement_nodes;
  }

  /*----------------*/
  /* Lone semicolon */
  /*----------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_SEMICOLON)) {
    if (m_print_debug) std::cout << "lone semicolon statement\n";
    return {AST_NODE_STATEMENT_EMPTY, {}, {}};
  }

  move_cursor_back_to(entry_cursor_pos);
  return {AST_NODE_NULL, {}, {}};
}

ASTNode Parser::expression() {
  int entry_cursor_pos{m_cursor_pos};

  /*--------------------------*/
  /* Parenthesised expression */
  /*--------------------------*/
  if (token(TOKEN_LPAREN)) {
    ASTNode expression_node{expression()};
    if (expression_node.type == AST_NODE_NULL) abort("Expected expression after '('");
    if (!token(TOKEN_RPAREN)) abort("Expected ')' after expression");

    // Greedily search for a binary operator
    std::string operator_name{binary_operator()};
    if (operator_name != "") {
      ASTNode next_expression_node{expression()};
      if (next_expression_node.type == AST_NODE_NULL) abort("Expected expression after operator");

      if (m_print_debug) std::cout << "operator expression\n";
      return {AST_NODE_EXPRESSION_BINARY_OPERATION,
              {{"type", operator_name}},
              {expression_node, next_expression_node}};
    }

    expression_node.data["parenthesised"] = "true";  // Mark this for precedence

    if (m_print_debug) std::cout << "parenthesised expression\n";
    return expression_node;
  }

  /*---------------------*/
  /* Negative expression */
  /*---------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_MINUS)) {
    ASTNode expression_node{expression()};
    if (expression_node.type == AST_NODE_NULL) abort("Expected expression after '-");

    if (m_print_debug) std::cout << "negative expression\n";
    return {AST_NODE_EXPRESSION_UNARY_OPERATION, {{"type", "minus"}}, {expression_node}};
  }

  /*--------------------*/
  /* Negated expression */
  /*--------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_NOT)) {
    ASTNode expression_node{expression()};
    if (expression_node.type == AST_NODE_NULL) abort("Expected expression after '!");

    if (m_print_debug) std::cout << "negated expression\n";
    return {AST_NODE_EXPRESSION_UNARY_OPERATION, {{"type", "not"}}, {expression_node}};
  }

  /*-------------------------------------*/
  /* Expression beginning with a literal */
  /*-------------------------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_FLOAT_LITERAL) || token(TOKEN_INT_LITERAL) || token(TOKEN_STRING_LITERAL)) {
    ASTNode literal_node{AST_NODE_EXPRESSION_LITERAL,
                         {{"type", Token::type_names.at(m_tokens[m_cursor_pos - 1].get_type())},
                          {"value", std::string{m_tokens[m_cursor_pos - 1].get_text()}}},
                         {}};

    // Greedily search for a binary operator
    std::string operator_name{binary_operator()};
    if (operator_name != "") {
      ASTNode next_expression_node{expression()};
      if (next_expression_node.type == AST_NODE_NULL) abort("Expected expression after operator");

      if (m_print_debug) std::cout << "operator expression\n";
      return {
          AST_NODE_EXPRESSION_BINARY_OPERATION, {{"type", operator_name}}, {literal_node, next_expression_node}};
    }

    // Only accept lone literal expression if no operator was found afterwards
    if (m_print_debug) std::cout << "literal expression\n";
    return literal_node;
  }

  /*------------------------------------------*/
  /* Expressions beginning with an identifier */
  /*------------------------------------------*/
  move_cursor_back_to(entry_cursor_pos);
  if (token(TOKEN_IDENTIFIER)) {
    /*--------------------------*/
    /* Function call identifier */
    /*--------------------------*/
    if (token(TOKEN_LPAREN)) {
      ASTNode function_call_node{
          AST_NODE_EXPRESSION_FUNCTION_CALL, {{"name", std::string{m_tokens[m_cursor_pos - 2].get_text()}}}, {}};

      std::vector<ASTNode> argument_expression_nodes{};
      argument_expression_nodes.push_back(expression());
      if (argument_expression_nodes.back().type != AST_NODE_NULL) {
        while (token(TOKEN_COMMA)) {
          argument_expression_nodes.push_back(expression());
          if (argument_expression_nodes.back().type == AST_NODE_NULL) abort("Expected expression after ','");
        }
        function_call_node.children.insert(function_call_node.children.begin(), argument_expression_nodes.begin(),
                                           argument_expression_nodes.end());
      }

      if (!token(TOKEN_RPAREN)) abort("Expected ')' at end of function call in statement");

      // Greedily search for a binary operator
      std::string operator_name{binary_operator()};
      if (operator_name != "") {
        ASTNode next_expression_node{expression()};
        if (next_expression_node.type == AST_NODE_NULL) abort("Expected expression after operator");

        if (m_print_debug) std::cout << "operator expression\n";
        return {AST_NODE_EXPRESSION_BINARY_OPERATION,
                {{"type", operator_name}},
                {function_call_node, next_expression_node}};
      }

      if (m_print_debug) std::cout << "function call expression\n";
      return function_call_node;
    }

    /*---------------------*/
    /* Variable identifier */
    /*---------------------*/
    ASTNode variable_node{
        AST_NODE_EXPRESSION_VARIABLE, {{"name", std::string{m_tokens[m_cursor_pos - 1].get_text()}}}, {}};

    // Greedily search for a binary operator
    std::string operator_name{binary_operator()};
    if (operator_name != "") {
      ASTNode next_expression_node{expression()};
      if (next_expression_node.type == AST_NODE_NULL) abort("Expected expression after operator");

      if (m_print_debug) std::cout << "operator expression\n";
      return {
          AST_NODE_EXPRESSION_BINARY_OPERATION, {{"type", operator_name}}, {variable_node, next_expression_node}};
    }

    if (m_print_debug) std::cout << "variable expression\n";
    return variable_node;
  }

  move_cursor_back_to(entry_cursor_pos);
  return {AST_NODE_NULL, {}, {}};
}

std::string Parser::binary_operator() {
  /*-----------------*/
  /* Binary operator */
  /*-----------------*/
  if (token(TOKEN_PLUS) || token(TOKEN_MINUS) || token(TOKEN_MULTIPLY) || token(TOKEN_DIVIDE) || token(TOKEN_EQ) ||
      token(TOKEN_NEQ) || token(TOKEN_LT) || token(TOKEN_LE) || token(TOKEN_GT) || token(TOKEN_GE) ||
      token(TOKEN_AND) || token(TOKEN_OR)) {
    if (m_print_debug) std::cout << "binary operator\n";
    return Token::type_names.at(m_tokens[m_cursor_pos - 1].get_type());
  }

  return "";
}

bool Parser::token(TokenType token_type) {
  /*-------*/
  /* Token */
  /*-------*/
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

Parser::Parser(Lexer &lexer, Emitter &emitter, bool print_debug)
    : m_lexer{lexer},
      m_emitter{emitter},
      m_tokens{},
      m_cursor_pos{0},
      m_print_debug{print_debug},
      m_ast_root{AST_NODE_NULL, {}, {}} {
  m_tokens.push_back(m_lexer.get_token());
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
  m_ast_root = program();

  if (m_ast_root.type != AST_NODE_NULL) m_emitter.write_file();

  if (m_print_debug) {
    std::cout << "\nProgram is valid: " << (m_ast_root.type != AST_NODE_NULL) << "\n";
    std::cout << "\n";
    m_ast_root.print_tree();
  }
}
