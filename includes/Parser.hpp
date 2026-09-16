/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 02:38:58 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 20:32:37 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

// #include "Compiler.hpp"
#include "Lexer.hpp"
#include <vector>

class Compiler;

struct AST {
  std::vector<struct AST> subNodes;
  t_token self;
};

class Parser {
private:
  AST self;

public:
  uint8_t parse_file(Compiler &global);

  void printAst();
};

#endif
