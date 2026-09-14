/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:22 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/15 01:09:23 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"
#include "Logging.hpp"

Lexer::Lexer(std::string &file) : _file(file) {}
Lexer::Lexer(std::string_view &file) : _file(file) {}

uint8_t Lexer::lex_file() {
  Logger::debug("Starting Lexer");
  return (0);
}
