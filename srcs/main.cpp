/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:05 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/21 05:55:58 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Compiler.hpp"
// #include "Lexer.hpp"
// #include "Logging.hpp"
// #include "FileReader.hpp"
// #include <format>
#include <algorithm>
#include <iostream>

int main(int ac, char **av) {
  if (ac < 2) {
    return (std::cout << "Welcome in the Glang Compiler version 0.3.0 !\n"
                      << "Usage : " << av[0] << " [files to compile]\n",
            0);
  }

  std::vector<std::string> paths;

  // NOTE: prevents fromreading the same file multiple times
  for (int k = 1; k < ac; k++) {
    if (std::find(paths.begin(), paths.end(), av[k]) == paths.end())
      paths.emplace_back(av[k]);
  }

  Compiler compiler;
  try {

    if (!compiler.lex_all(paths)) {
      std::cerr << "Error while lexing files.\n";
      return (1);
    }
    if (!compiler.parse_all(paths)) {
      std::cerr << "Error while parsing files.\n";
      return (1);
    }
    if (!compiler.analyze_all(paths)) {
      std::cerr << "Error while analyzing files.\n";
      return (1);
    }
  } catch (const std::exception &e) {
    std::cerr << e.what();
  }
  return (0);
}
