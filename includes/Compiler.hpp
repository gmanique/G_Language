/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:48:41 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 02:02:47 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "Lexer.hpp"
// #include "Parser.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

struct SourceFile {
  std::string path;
  std::string content;
  std::unique_ptr<Lexer> lexer;
  // std::unique_ptr<Parser> parser;
};

class Compiler {
private:
  std::unordered_map<std::string, SourceFile> _files;

public:
  bool lex_all(const std::vector<std::string> &paths);
};

#endif
