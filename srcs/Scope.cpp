/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Scope.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 05:26:15 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 06:37:32 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scope.hpp"
#include "Compiler.hpp"

Scope::Scope() {}
Scope::Scope(Scope &father) {
  this->vars = father.getVars();
  this->imports = father.getImports();
}
Scope::~Scope() {}

const std::vector<VarDef> &Scope::getVars() { return this->vars; }
const std::vector<std::string> &Scope::getImports() { return this->imports; }

void Scope::addImport(std::string &imp) {
  this->imports.push_back(std::string(imp));
}
