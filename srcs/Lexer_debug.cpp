/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer_debug.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 07:04:16 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 20:25:58 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"
#include <iostream>

constexpr std::string_view token_id_to_string(t_tokenID id) {
  switch (id) {
  case KW_IMPORT:
    return "KW_IMPORT";
  case KW_SYSCALL:
    return "KW_SYSCALL";
  case KW_FUN:
    return "KW_FUN";
  case KW_TYPE:
    return "KW_TYPE";
  case KW_ENUM:
    return "KW_ENUM";
  case KW_IF:
    return "KW_IF";
  case KW_ELSE:
    return "KW_ELSE";
  case KW_WHILE:
    return "KW_WHILE";
  case KW_STRUCT:
    return "KW_STRUCT";
  case KW_RETURN:
    return "KW_RETURN";
  case CHAR:
    return "CHAR";
  case WORD:
    return "WORD";
  case NUMBER:
    return "NUMBER";
  case STRING:
    return "STRING";
  case OPERATOR:
    return "OPERATOR";
  case SEPARATOR:
    return "SEPARATOR";
  case EOI:
    return "EOI";
  case EOFF:
    return "EOF";
  default:
    return "UNKNOWN";
  }
}
std::ostream &operator<<(std::ostream &os, const t_token &tok) {
  os << "Token(" << token_id_to_string(tok.id) << ", \"" << tok.value << "\")";
  return os;
}
std::ostream &operator<<(std::ostream &os, const std::list<t_token> &tokens) {
  os << "[\n";
  for (const auto &tok : tokens) {
    os << "  " << tok << "\n";
  }
  os << "]";
  return os;
}
void Lexer::Print() {
  std::cout << "Lex : ";
  std::cout << _tokens << "\n";
}
