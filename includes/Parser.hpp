/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 02:38:58 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 21:19:57 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

// #include "Compiler.hpp"
#include "Lexer.hpp"
#include <concepts>
#include <stdexcept>
#include <string>
#include <vector>

class Compiler;
struct SourceFile;

// NOTE: Generic parse-tree node. `self` is the token that "represents" the
// node (the keyword/operator/identifier that introduced it), and `subNodes`
// are its children in a grammar-rule-dependent, but FIXED, order. The exact
// order used by each parse_XXX() function is documented above that function
// in Parser.cpp. This stays untyped on purpose: semantic analysis (0.3.0)
// is the layer that will walk this tree and turn it into something typed.
struct AST {
  std::vector<struct AST> subNodes;
  t_token self;
};

class Parser {
private:
  AST self;

  std::vector<t_token> _toks;
  size_t _pos = 0;
  std::string _path;

  const t_token &peek(size_t offset = 0) const;
  const t_token &advance();
  bool check(t_tokenID id) const;
  bool checkValue(std::string_view v) const;
  const t_token &expect(t_tokenID id, const char *what);
  const t_token &expectValue(std::string_view v, const char *what);
  [[noreturn]] void error(const std::string &msg) const;

  static AST leaf(const t_token &tok);
  static AST node(const t_token &tok, std::vector<AST> children);

  // NOTE: Builds a node from AST rvalues WITHOUT copying them. Do not write
  // node(tok, {std::move(a), std::move(b)}): elements of an initializer_list
  // are const, so the vector constructor deep-copies them (this made parsing
  // quadratic on long expressions such as 1+1+1+...).
  template <std::same_as<AST>... Cs>
  static AST nodeOf(const t_token &tok, Cs &&...cs) {
    std::vector<AST> v;
    v.reserve(sizeof...(Cs));
    (v.push_back(std::move(cs)), ...);
    return AST{std::move(v), tok};
  }

  // NOTE: Recursion limit. Without it, a few thousand nested '(' or '{'
  // overflow the C++ stack (segfault) instead of giving a parse error.
  static constexpr size_t MAX_DEPTH = 512;
  size_t _depth = 0;
  struct DepthGuard {
    Parser &p;
    explicit DepthGuard(Parser &parser) : p(parser) {
      if (++p._depth > MAX_DEPTH) {
        --p._depth; // NOTE: the destructor won't run if we throw here
        p.error("nesting too deep");
      }
    }
    ~DepthGuard() { --p._depth; }
    DepthGuard(const DepthGuard &) = delete;
    DepthGuard &operator=(const DepthGuard &) = delete;
  };

  AST parseProgram();
  AST parseTopLevel();
  AST parseImport();
  AST parseStructDecl();
  AST parseEnumDecl();
  AST parseFunDecl();
  AST parseParamList();
  AST parseType();

  AST parseBlock();
  AST parseStatement();
  bool looksLikeVarDecl();
  AST parseVarDecl();
  AST parseIfStmt();
  AST parseWhileStmt();
  AST parseReturnStmt();
  AST parseExprStmt();

  AST parseExpr();
  AST parseAssignment();
  AST parseLogicalOr();
  AST parseLogicalAnd();
  AST parseEquality();
  AST parseRelational();
  AST parseAdditive();
  AST parseMultiplicative();
  AST parseUnary();
  AST parsePostfix();
  AST parsePrimary();
  AST parseArgList();

public:
  uint8_t parse_file(Compiler &global, SourceFile &file);
  struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
  const AST &getAst() const { return self; }

  void printAst();
};

#endif
