/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scope.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 05:25:37 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 06:37:23 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCOPE_HPP
#define SCOPE_HPP

#include <concepts>
#include <string>
#include <vector>

class Parser;
struct FuncDef;
struct VarDef;
struct StructDef;

class Scope {
private:
  std::vector<VarDef> vars;
  std::vector<std::string> imports;

public:
  Scope();
  Scope(Scope &father);
  ~Scope();
  const std::vector<VarDef> &getVars();
  const std::vector<std::string> &getImports();

  void addImport(std::string &imp);

  template <std::constructible_from<std::string> T1,
            std::constructible_from<std::string> T2>
  void addVarDef(T1 &&name, T2 &&type) {
    vars.emplace_back(std::forward<T1>(name), std::forward<T2>(type));
  }
};

#endif
