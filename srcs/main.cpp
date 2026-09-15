/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:05 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/15 07:23:48 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "Logging.hpp"
#include <FileReader.hpp>
// #include <format>
#include "Lexer.hpp"
#include <iostream>

int main(int ac, char **av) {
  if (ac < 2)
    return (std::cout << "Welcome in the Glang Compiler version 0.1.0 !\n", 0);
  if (ac > 2)
    return (std::cout << "I can only handle one file at once yet.\n", 0);

  std::string path(av[1]);
  FileReader a;
  std::optional<std::string> tmp = a.read_file(path);
  if (!tmp.has_value()) {
    return (std::cout << "Empty or inexistant file", 1);
  }

  std::cout << tmp.value();

  Lexer lex(tmp.value());
  uint8_t status;
  if ((status = lex.lex_file())) {
    return (status);
  }
  lex.Print();

  return (0);
}
