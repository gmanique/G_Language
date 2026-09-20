/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 02:39:28 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 21:36:47 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Compiler.hpp"

#include <iostream>

const t_token &Parser::peek(size_t offset) const {
  size_t idx = _pos + offset;
  if (idx >= _toks.size())
    return _toks.back();
  return _toks[idx];
}

const t_token &Parser::advance() {
  const t_token &tok = peek();
  if (_pos < _toks.size() - 1)
    _pos++;
  return tok;
}

bool Parser::check(t_tokenID id) const { return peek().id == id; }

bool Parser::checkValue(std::string_view v) const { return peek().value == v; }

const t_token &Parser::expect(t_tokenID id, const char *what) {
  if (!check(id))
    error(std::string("expected ") + what + " but got \"" +
          std::string(peek().value) + "\"");
  return advance();
}

const t_token &Parser::expectValue(std::string_view v, const char *what) {
  if (!checkValue(v))
    error(std::string("expected ") + what + " but got \"" +
          std::string(peek().value) + "\"");
  return advance();
}

void Parser::error(const std::string &msg) const {
  const t_token &tok = peek();
  throw ParseError(_path + ":" + std::to_string(tok.line) + ":" +
                   std::to_string(tok.col) + ": " + msg);
}

AST Parser::leaf(const t_token &tok) { return AST{{}, tok}; }

AST Parser::node(const t_token &tok, std::vector<AST> children) {
  return AST{std::move(children), tok};
}

// TODO: Add every element to self sourcefile for later syntax analysis
uint8_t Parser::parse_file(Compiler &global, SourceFile &file) {
  (void)global;
  _path = file.path;
  _toks.assign(file.lexer->get_tokens().begin(),
               file.lexer->get_tokens().end());
  _pos = 0;
  _depth = 0;
  if (_toks.empty()) {
    t_token eof{};
    eof.id = EOFF;
    _toks.push_back(eof);
  }

  try {
    self = parseProgram();
  } catch (const ParseError &e) {
    std::cerr << "Parse error: " << e.what() << "\n";
    return (1);
  }
  return (0);
}

AST Parser::parseProgram() {
  t_token root{};
  root.id = EOFF;
  root.value = "Program";
  std::vector<AST> decls;
  while (!check(EOFF))
    decls.push_back(parseTopLevel());
  return (node(root, std::move(decls)));
}

AST Parser::parseTopLevel() {
  if (check(KW_IMPORT))
    return parseImport();
  if (check(KW_STRUCT))
    return parseStructDecl();
  if (check(KW_ENUM))
    return parseEnumDecl();
  if (check(KW_FUN))
    return parseFunDecl();
  error("expected 'import', 'struct', 'enum' or 'fun' at top level");
}

AST Parser::parseImport() {
  t_token kw = advance();
  AST path = leaf(expect(STRING, "a string (import path)"));
  if (check(EOI))
    advance();
  return nodeOf(kw, std::move(path));
}

AST Parser::parseType() {
  t_token base;
  if (check(KW_TYPE) || check(WORD))
    base = advance();
  else
    error("expected a type name");

  std::vector<AST> mods;
  while (check(OPERATOR) &&
         peek().value.find_first_not_of('*') == std::string_view::npos) {
    mods.push_back(leaf(advance()));
  }
  if (check(SEPARATOR) && checkValue("[")) {
    t_token bracket = advance(); // "["
    std::vector<AST> bracketChildren;
    if (check(NUMBER))
      bracketChildren.push_back(leaf(advance()));
    expectValue("]", "closing ']' in array type");
    mods.push_back(node(bracket, std::move(bracketChildren)));
  }
  return node(base, std::move(mods));
}

AST Parser::parseStructDecl() {
  t_token kw = advance(); // "struct"
  AST name = leaf(expect(WORD, "a struct name"));
  expectValue("{", "'{' to open struct body");

  std::vector<AST> children;
  children.push_back(std::move(name));
  while (!(check(SEPARATOR) && checkValue("}"))) {
    AST type = parseType();
    t_token fieldName = expect(WORD, "a field name");
    expect(EOI, "';' after struct field");
    children.push_back(nodeOf(fieldName, std::move(type)));
  }
  expectValue("}", "'}' to close struct body");
  return node(kw, std::move(children));
}

AST Parser::parseEnumDecl() {
  t_token kw = advance(); // "enum"
  AST name = leaf(expect(WORD, "an enum name"));
  expectValue("{", "'{' to open enum body");

  std::vector<AST> children;
  children.push_back(std::move(name));
  while (!(check(SEPARATOR) && checkValue("}"))) {
    children.push_back(leaf(expect(WORD, "an enum member")));
    if (check(SEPARATOR) && checkValue(","))
      advance();
    else
      break;
  }
  expectValue("}", "'}' to close enum body");
  return node(kw, std::move(children));
}

AST Parser::parseFunDecl() {
  t_token kw = advance(); // "fun"
  AST name = leaf(expect(WORD, "a function name"));
  expectValue("(", "'(' to open parameter list");
  AST params = parseParamList();
  expectValue(")", "')' to close parameter list");

  std::vector<AST> children;
  children.push_back(std::move(name));
  children.push_back(std::move(params));
  if (!(check(SEPARATOR) && checkValue("{")))
    children.push_back(parseType());
  children.push_back(parseBlock());
  return node(kw, std::move(children));
}

AST Parser::parseParamList() {
  t_token marker{};
  marker.id = SEPARATOR;
  marker.value = "(";
  std::vector<AST> params;
  if (!(check(SEPARATOR) && checkValue(")"))) {
    do {
      AST type = parseType();
      t_token pname = expect(WORD, "a parameter name");
      params.push_back(nodeOf(pname, std::move(type)));
    } while (check(SEPARATOR) && checkValue(",") && (advance(), true));
  }
  return node(marker, std::move(params));
}

AST Parser::parseBlock() {
  DepthGuard guard(*this);
  t_token open = expectValue("{", "'{' to open a block");
  std::vector<AST> stmts;
  while (!(check(SEPARATOR) && checkValue("}")))
    stmts.push_back(parseStatement());
  expectValue("}", "'}' to close a block");
  return node(open, std::move(stmts));
}

bool Parser::looksLikeVarDecl() {
  if (check(KW_TYPE))
    return true;
  if (!check(WORD))
    return false;
  size_t save = _pos;
  bool ok = false;
  try {
    parseType();
    ok = check(WORD);
  } catch (const ParseError &) {
    ok = false;
  }
  _pos = save;
  return ok;
}

AST Parser::parseStatement() {
  if (check(SEPARATOR) && checkValue("{"))
    return parseBlock();
  if (check(KW_IF))
    return parseIfStmt();
  if (check(KW_WHILE))
    return parseWhileStmt();
  if (check(KW_RETURN))
    return parseReturnStmt();
  if (checkValue("let") || checkValue("const") || looksLikeVarDecl())
    return parseVarDecl();
  return parseExprStmt();
}

AST Parser::parseVarDecl() {
  t_token qualifier{};
  bool hasQualifier = checkValue("let") || checkValue("const");
  if (hasQualifier)
    qualifier = advance();

  AST type = parseType();
  t_token name = expect(WORD, "a variable name");
  std::vector<AST> children;
  children.push_back(std::move(type));
  children.push_back(leaf(name));
  if (check(OPERATOR) && checkValue("=")) {
    advance();
    children.push_back(parseExpr());
  }
  expect(EOI, "';' after variable declaration");
  t_token selfTok = hasQualifier ? qualifier : children[0].self;
  return node(selfTok, std::move(children));
}

AST Parser::parseIfStmt() {
  DepthGuard guard(*this);
  t_token kw = advance(); // "if"
  expectValue("(", "'(' after 'if'");
  AST cond = parseExpr();
  expectValue(")", "')' after if-condition");
  AST thenBlock = parseBlock();

  std::vector<AST> children;
  children.push_back(std::move(cond));
  children.push_back(std::move(thenBlock));
  if (check(KW_ELSE)) {
    advance();
    children.push_back(check(KW_IF) ? parseIfStmt() : parseBlock());
  }
  return node(kw, std::move(children));
}

AST Parser::parseWhileStmt() {
  t_token kw = advance(); // "while"
  expectValue("(", "'(' after 'while'");
  AST cond = parseExpr();
  expectValue(")", "')' after while-condition");
  AST body = parseBlock();
  return nodeOf(kw, std::move(cond), std::move(body));
}

AST Parser::parseReturnStmt() {
  t_token kw = advance(); // "return"
  std::vector<AST> children;
  if (!check(EOI))
    children.push_back(parseExpr());
  expect(EOI, "';' after return statement");
  return node(kw, std::move(children));
}

AST Parser::parseExprStmt() {
  AST e = parseExpr();
  expect(EOI, "';' after expression");
  return e;
}

AST Parser::parseExpr() { return parseAssignment(); }

AST Parser::parseAssignment() {
  DepthGuard guard(*this);
  AST lhs = parseLogicalOr();
  if (check(OPERATOR) && checkValue("=")) {
    t_token op = advance();
    AST rhs = parseAssignment(); // right-associative
    return nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseLogicalOr() {
  AST lhs = parseLogicalAnd();
  while (check(OPERATOR) && checkValue("||")) {
    t_token op = advance();
    AST rhs = parseLogicalAnd();
    lhs = nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseLogicalAnd() {
  AST lhs = parseEquality();
  while (check(OPERATOR) && checkValue("&&")) {
    t_token op = advance();
    AST rhs = parseEquality();
    lhs = nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseEquality() {
  AST lhs = parseRelational();
  while (check(OPERATOR) && (checkValue("==") || checkValue("!="))) {
    t_token op = advance();
    AST rhs = parseRelational();
    lhs = nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseRelational() {
  AST lhs = parseAdditive();
  while (check(OPERATOR) && (checkValue("<") || checkValue(">") ||
                             checkValue("<=") || checkValue(">="))) {
    t_token op = advance();
    AST rhs = parseAdditive();
    lhs = nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseAdditive() {
  AST lhs = parseMultiplicative();
  while (check(OPERATOR) && (checkValue("+") || checkValue("-"))) {
    t_token op = advance();
    AST rhs = parseMultiplicative();
    lhs = nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseMultiplicative() {
  AST lhs = parseUnary();
  while (check(OPERATOR) &&
         (checkValue("*") || checkValue("/") || checkValue("%"))) {
    t_token op = advance();
    AST rhs = parseUnary();
    lhs = nodeOf(op, std::move(lhs), std::move(rhs));
  }
  return lhs;
}

AST Parser::parseUnary() {
  DepthGuard guard(*this);
  if (check(OPERATOR) && (checkValue("!") || checkValue("-") ||
                          checkValue("&") || checkValue("*"))) {
    t_token op = advance();
    AST operand = parseUnary();
    return nodeOf(op, std::move(operand));
  }
  return parsePostfix();
}

AST Parser::parsePostfix() {
  AST expr = parsePrimary();
  while (true) {
    if (check(SEPARATOR) && checkValue("(")) {
      t_token open = advance();
      AST args = parseArgList();
      expectValue(")", "')' to close a call");
      std::vector<AST> children;
      children.push_back(std::move(expr));
      children.push_back(std::move(args));
      expr = node(open, std::move(children));
    } else if (check(SEPARATOR) && checkValue("[")) {
      t_token open = advance();
      AST index = parseExpr();
      expectValue("]", "']' to close an index");
      expr = nodeOf(open, std::move(expr), std::move(index));
    } else if (check(SEPARATOR) && checkValue(".")) {
      t_token dot = advance();
      AST member = leaf(expect(WORD, "a member name"));
      expr = nodeOf(dot, std::move(expr), std::move(member));
    } else {
      break;
    }
  }
  return expr;
}

AST Parser::parseArgList() {
  t_token marker{};
  marker.id = SEPARATOR;
  marker.value = "(";
  std::vector<AST> args;
  if (!(check(SEPARATOR) && checkValue(")"))) {
    do {
      args.push_back(parseExpr());
    } while (check(SEPARATOR) && checkValue(",") && (advance(), true));
  }
  return node(marker, std::move(args));
}

AST Parser::parsePrimary() {
  if (check(NUMBER) || check(STRING) || check(CHAR) || check(WORD))
    return leaf(advance());

  if (check(KW_SYSCALL)) {
    t_token kw = advance();
    expectValue("(", "'(' after 'syscall'");
    AST args = parseArgList();
    expectValue(")", "')' to close syscall arguments");
    return nodeOf(kw, std::move(args));
  }

  if (check(SEPARATOR) && checkValue("(")) {
    advance();
    AST inner = parseExpr();
    expectValue(")", "')' to close a parenthesised expression");
    return inner;
  }

  error("expected an expression");
}
