/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:54:06 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 03:54:04 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
#include "FileReader.hpp"
#include "Parser.hpp"

SourceFile::SourceFile() = default;
SourceFile::~SourceFile() = default;
SourceFile::SourceFile(SourceFile &&) noexcept = default;
SourceFile &SourceFile::operator=(SourceFile &&) noexcept = default;

// Compiler::Compiler() = default;
// Compiler::~Compiler() = default;

SourceFile &Compiler::get_file(const std::string &name) { return _files[name]; }

// Reste de tes méthodes Compiler...
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

void add_funcs(SourceFile &file) {
  const std::vector<AST> &nodes = file.parser->getAst().subNodes;
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].self.id == KW_FUN) {
      FuncDef func;
      func.FuncName = nodes[i].subNodes[0].self.value;
      for (size_t j = 0; j < nodes[i].subNodes[1].subNodes.size(); j++) {
        func.Args.push_back(
            std::string(nodes[i].subNodes[1].subNodes[j].self.value));
      }
      func.returnType = nodes[i].subNodes[2].self.value;
      file.definedFuncs.push_back(func);
    }
  }
}

void add_structs(SourceFile &file) {
  const std::vector<AST> &nodes = file.parser->getAst().subNodes;
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].self.id == KW_STRUCT) {
      StructDef curr;
      curr.StructName = nodes[i].subNodes[0].self.value;
      const std::vector<AST> &subTree = nodes[i].subNodes;
      for (size_t j = 1; j < subTree.size(); j++) {
        VarDef var;
        var.name = subTree[i].self.value;
        var.type = subTree[i]
                       .subNodes[0]
                       .self.value; // NOTE: Maybe not because of pointers ? Idk
                                    // how to handle it yet
        curr.Args.push_back(var);
      }
      file.definedStructTypes.push_back(curr);
    }
  }
}

void add_imports(SourceFile &file) {
  const std::vector<AST> &nodes = file.parser->getAst().subNodes;
  for (size_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].self.id == KW_IMPORT) {
      std::string_view rawPath = nodes[i].subNodes[0].self.value;
      if (rawPath.size() >= 2 && rawPath.front() == '"' &&
          rawPath.back() == '"') {
        rawPath.remove_prefix(1);
        rawPath.remove_suffix(1);
      }
      file.importedFiles.push_back(std::string(rawPath));
    }
  }
}

bool Compiler::parse_all(const std::vector<std::string> &paths) {

  for (const auto &path : paths) {
    SourceFile &file = _files[path];

    file.parser = std::make_unique<Parser>();
    if (file.parser->parse_file(*this, file) != 0) {
      return false;
    }
    add_imports(file);
    add_structs(file);
    // add_enums(file); // Potentiellement en meme temps que struts, quand je
    // gererai des enums si j'en gere ?
    add_funcs(file);
  }

  // NOTE: Delete this print when done
  for (const auto &path : paths) {
    _files[path].parser->printAst();
  }

  return true;
}
