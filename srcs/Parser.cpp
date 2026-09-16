/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 02:39:28 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 22:52:58 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Compiler.hpp"

#include <iostream>

uint8_t Parser::parse_file(Compiler &global, SourceFile &file) {
  (void)global;
  std::list<t_token> toks = file.lexer->get_tokens();

  for (auto it = toks.begin(); it != toks.end(); ++it) {
    std::cout << (*it).value << " ";
    t_token curr = *it;
    (void)curr;
    // switch (curr.id) { ; }
  }
  return (0);
}
