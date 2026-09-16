/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:48:41 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 22:39:48 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMPILER_HPP
#define COMPILER_HPP

#include "Lexer.hpp"
// #include "Parser.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class Parser;

// NOTE: Sert a lister les definitions de fonction par fichier pour savoir si
// telle ou telle fonction incluse de tel fichier existe et correspond
struct FuncDef {
  std::string FuncName;
  std::string returnType;
  std::vector<std::string> Args; // NOTE: ex : "i8", "string", "u32"
};

struct SourceFile {
  std::string path;
  std::string content;
  std::unique_ptr<Lexer> lexer;
  std::unique_ptr<Parser> parser;
  std::vector<std::string> definedTypes;
  std::vector<std::string> definedFuncs;

  SourceFile();
  ~SourceFile();

  SourceFile(SourceFile &&) noexcept;
  SourceFile &operator=(SourceFile &&) noexcept;
};

class Compiler {
private:
  std::unordered_map<std::string, SourceFile> _files;

public:
  SourceFile &get_file(const std::string &name);
  bool lex_all(const std::vector<std::string> &paths);
  bool parse_all(const std::vector<std::string> &paths);
};

#endif
