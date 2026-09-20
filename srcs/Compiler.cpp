/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:54:06 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 06:39:48 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
#include "FileReader.hpp"
#include "Parser.hpp"
#include "Scope.hpp"

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
    // add_enums(file); // Potentiellement en meme temps que structs, quand je
    // gererai des enums si j'en gere ?
    add_funcs(file);
  }

  // NOTE: Delete this print when done
  for (const auto &path : paths) {
    _files[path].parser->printAst();
  }

  return true;
}

bool inScope(const std::string_view &elem, Scope &scope) {
  const std::vector<VarDef> &variables = scope.getVars();
  size_t idx = 0;
  uint8_t found = 0;
  for (size_t i = 0; i < variables.size(); i++) {
    if (elem == variables[i].name) {
      idx = i;
      found = 1;
      break;
    }
  }
  if (found == 0)
    return false;
  // TODO: verifier que l'element ensuite va bien a la variable
  (void)idx;
  return true;
}

bool Compiler::analyze_block(const AST &node, Scope &scope) {
  Scope curr_scope(scope);
  if (node.self.id == KW_TYPE) {
    if (inScope(node.self.value, curr_scope))
      return false; // NOTE: Redefinition of element
    scope.addVarDef(node.subNodes[0].self.value, node.self.value);
    if (node.subNodes.size() > 1) {
      // TODO: check that type is correct
    }
  } else if (node.self.id == WORD) {
    if (!inScope(node.self.value, curr_scope)) {
      return false; // NOTE: Element never defined
    }
  }
  return true;
}

bool Compiler::analyze_all(const std::vector<std::string> &paths) {
  for (const auto &path : paths) {
    SourceFile &file = _files[path];
    const std::vector<AST> &nodes = file.parser->getAst().subNodes;
    Scope scope;
    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i].self.id == KW_IMPORT)
        continue;

      if (analyze_block(nodes[i], scope) == false) {
        throw std::runtime_error("Issue");
        return false;
      }
    }
  }
  return true;
}
