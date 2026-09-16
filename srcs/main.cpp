/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:05 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 22:46:13 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
// #include "Lexer.hpp"
// #include "Logging.hpp"
// #include "FileReader.hpp"
// #include <format>
#include <iostream>

int main(int ac, char **av) {
  if (ac < 2) {
    return (std::cout << "Welcome in the Glang Compiler version 0.2.0 !\n"
                      << "Usage : " << av[0] << " [files to compile]\n",
            0);
  }

  std::vector<std::string> paths(av + 1, av + ac);

  Compiler compiler;
  if (!compiler.lex_all(paths)) {
    std::cerr << "Error while lexing files.\n";
    return (1);
  }
  if (!compiler.parse_all(paths)) {
    std::cerr << "Error while parsing files.\n";
    return (1);
  }
  return (0);
}
