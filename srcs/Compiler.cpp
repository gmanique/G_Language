/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:54:06 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 20:17:12 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
#include "FileReader.hpp"

bool Compiler::lex_all(const std::vector<std::string> &paths) {

  FileReader reader;

  for (const auto &path : paths) {
    auto content = reader.read_file(path);
    if (!content)
      return false;

    SourceFile &file = _files[path];
    file.path = path;
    file.content = std::move(*content); // Adresse 100% stable

    file.lexer = std::make_unique<Lexer>(file.content);

    if (file.lexer->lex_file() != 0) {
      return false;
    }
  }

  // NOTE: Delete this print when done
  for (const auto &path : paths) {
    _files[path].lexer->Print();
  }
  return true;
}

bool Compiler::parse_all(const std::vector<std::string> &paths) {

  for (const auto &path : paths) {
    SourceFile &file = _files[path];

    if (file.parser->parse_file(*this) != 0) {
      return false;
    }
  }

  // NOTE: Delete this print when done
  for (const auto &path : paths) {
    _files[path].lexer->Print();
  }
  return true;
}
