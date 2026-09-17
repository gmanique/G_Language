/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser_debug.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 02:40:13 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/18 00:32:57 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include <iostream>

static void printNode(const AST &node, int depth) {
  std::cout << std::string(depth * 2, ' ') << "\"" << node.self.value << "\""
            << " (" << (int)node.self.id << ")\n";
  for (const auto &child : node.subNodes)
    printNode(child, depth + 1);
}

void Parser::printAst() {
  std::cout << "AST:\n";
  printNode(self, 0);
}
