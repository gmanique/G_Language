/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Compiler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 01:54:06 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/22 14:59:41 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
#include "FileReader.hpp"
#include "Parser.hpp"
#include "Scope.hpp"
#include <iostream>

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
    if (!content) {
      std::cerr << "Cannot read '" << path
                << "' (missing, not a regular file, or unreadable).\n";
      return false;
    }

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
        var.name = subTree[j].self.value;
        var.type = subTree[j]
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
  uint8_t found = 0;
  for (size_t i = 0; i < variables.size(); i++) {
    if (elem == variables[i].name) {
      found = 1;
      break;
    }
  }
  if (found == 0)
    return false;
  return true;
}

bool Compiler::checkFuncArgs(const AST &node, Scope &scope,
                             std::string &fileName) {
  for (size_t i = 0; i < node.subNodes.size(); i++) {
    if (node.subNodes[i].self.id != WORD) {
      std::cerr << "Error: '" << node.subNodes[i].self.value
                << "' Incorrect variable name at " << node.subNodes[i].self.line
                << ":" << node.subNodes[i].self.col << ".\n";
      return false;
    }
    if (inScope(node.subNodes[i].self.value, scope)) {
      std::cerr << "Error: '" << node.subNodes[i].self.value
                << "' defined twice at " << node.subNodes[i].self.line << ":"
                << node.subNodes[i].self.col << ".\n";
      return false;
    }
    if (node.subNodes[i].subNodes.size() != 1) {
      std::cerr << "Error: '" << node.subNodes[i].self.value
                << "' doesn't have a type at " << node.subNodes[i].self.line
                << ":" << node.subNodes[i].self.col << ".\n";
      return false;
    }
    if (!isaType(node.subNodes[i].subNodes[0].self.value, fileName)) {
      std::cerr << "Error: '" << node.subNodes[i].subNodes[0].self.value
                << "' incorrect type at "
                << node.subNodes[i].subNodes[0].self.line << ":"
                << node.subNodes[i].subNodes[0].self.col << ".\n";
      return false;
    }
    scope.addVarDef(node.subNodes[i].self.value,
                    node.subNodes[i].subNodes[0].self.value);
  }
  return true;
}

// TODO: Verifier que l'elem est bien un type defini (par defaut, dans le
// fichier ou les fichiers importes)

// DONE ?
bool Compiler::isaType(std::string_view elem, std::string &fileName) {
  if (elem == "i8" || elem == "i16" || elem == "i32" || elem == "i64" ||
      elem == "i128" || elem == "u8" || elem == "u16" || elem == "u32" ||
      elem == "u64" || elem == "u128" || elem == "string" || elem == "void")
    return (true);
  SourceFile &self = get_file(fileName);
  for (StructDef curr : self.definedStructTypes) {
    if (curr.StructName == elem)
      return (true);
  }
  for (std::string_view currFile : self.importedFiles) {
    SourceFile &srcCurr = get_file(std::string(currFile));
    for (StructDef curr : srcCurr.definedStructTypes) {
      if (curr.StructName == elem)
        return (true);
    }
  }
  return (false);
}

// TODO: Checker que la fonction ne soit pas deja definie
bool Compiler::alreadyDeclaredFunc(std::string_view funcName,
                                   std::string &fileName) {
  (void)funcName;
  (void)fileName;
  return false;
}

bool Compiler::analyze_func(const AST &node, Scope &scope,
                            std::string &fileName) {
  if (alreadyDeclaredFunc(node.subNodes[0].self.value, fileName)) {
    std::cerr << "Error: Function declared twice `"
              << node.subNodes[0].self.value << "` at "
              << node.subNodes[0].self.line << ":" << node.subNodes[0].self.col
              << ".\n";
    return false;
  }
  if (!checkFuncArgs(node.subNodes[1], scope, fileName)) {
    return false;
  }
  if (node.subNodes[2].self.value == "{") {
    return (analyze_block(node.subNodes[2], scope, fileName));
  }
  if (!isaType(node.subNodes[2].self.value, fileName)) {
    std::cerr << "Error: Incorrect type `" << node.subNodes[2].self.value
              << "` at " << node.subNodes[2].self.line << ":"
              << node.subNodes[2].self.col << ".\n";
    return false;
  }
  return (analyze_block(node.subNodes[3], scope, fileName));
}

bool Compiler::analyze_struct(const AST &node, Scope &scope,
                              std::string &fileName) {
  (void)node;
  (void)scope;
  (void)fileName;
  return true;
}

bool Compiler::analyze_block(const AST &node, Scope &scope,
                             std::string &fileName) {
  Scope curr_scope(scope);

  // NOTE: Supprimer ca quand jai fini
  // std::cout << "currently checking `" << node.self.value;
  // if (node.subNodes.size() > 0)
  //   std::cout << " " << node.subNodes[0].self.value;
  // std::cout << "`" << std::endl;

  if (node.self.id == KW_FUN) {
    if (!analyze_func(node, curr_scope, fileName))
      return false;
  } else if (node.self.id == KW_STRUCT) {
    if (!analyze_struct(node, curr_scope, fileName)) {
      return false;
    }
  } else if (node.self.id == KW_TYPE) {
    if (inScope(node.subNodes[1].self.value, curr_scope)) {
      return false; // NOTE: Redefinition of element
    }
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
  try {
    for (const auto &path : paths) {
      SourceFile &file = _files[path];
      const std::vector<AST> &nodes = file.parser->getAst().subNodes;
      Scope scope;
      for (size_t i = 0; i < nodes.size(); i++) {

        // std::cout << "currently checking `" << nodes[i].self.value;
        // if (nodes[i].subnodes.size() > 0)
        //   std::cout << " " << nodes[i].subnodes[0].self.value;
        // std::cout << "`" << std::endl;

        if (nodes[i].self.id == KW_IMPORT)
          continue;
        std::string currPath = path;
        if (analyze_block(nodes[i], scope, currPath) == false) {
          throw std::runtime_error("Issue");
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}
