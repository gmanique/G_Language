/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:54:06 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 02:05:41 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
#include "FileReader.hpp"

bool Compiler::lex_all(const std::vector<std::string> &paths) {
  for (const auto &path : paths) {
    SourceFile file;
    file.path = path;

    FileReader reader;
    auto content = reader.read_file(path);
    if (!content)
      return false;

    file.content = std::move(*content);
    file.lexer = std::make_unique<Lexer>(file.content);

    if (file.lexer->lex_file() != 0) {
      return false;
    }

    // NOTE: Delete when done
    file.lexer->Print();

    _files[path] = std::move(file);
  }
  return true;
}
