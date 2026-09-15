/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:22 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/15 05:39:47 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"
#include <exception>
#include <iostream>

Lexer::Lexer(std::string &file) : _file(file), i(0), line(1), col(1) {}
Lexer::Lexer(std::string_view &file) : _file(file), i(0), line(1), col(1) {}

// constexpr std::string_view token_id_to_string(t_tokenID id) {
//   switch (id) {
//   case WORD:
//     return "WORD";
//   case NUMBER:
//     return "NUMBER";
//   case STRING:
//     return "STRING";
//   case OPERATOR:
//     return "OPERATOR";
//   case SEPARATOR:
//     return "SEPARATOR";
//   case EOI:
//     return "EOI";
//   case EOFF:
//     return "EOF";
//   default:
//     return "UNKNOWN";
//   }
// }
// std::ostream &operator<<(std::ostream &os, const t_token &tok) {
//   os << "Token(" << token_id_to_string(tok.id) << ", \"" << tok.value <<
//   "\")"; return os;
// }
// std::ostream &operator<<(std::ostream &os, const std::list<t_token> &tokens)
// {
//   os << "[\n";
//   for (const auto &tok : tokens) {
//     os << "  " << tok << "\n";
//   }
//   os << "]";
//   return os;
// }
// void Lexer::Print() {
//   std::cout << "Lex : ";
//   std::cout << _tokens;
// }

static uint8_t ft_isoperator(char c) {
  return (c == '+' || c == '-' || c == '&' || c == '|' || c == '*' ||
          c == '/' || c == '%' || c == '=' || c == '!' || c == '<' || c == '>');
}

static uint8_t ft_isseparator(char c) {
  return (c == '(' || c == '{' || c == '[' || c == ')' || c == '}' ||
          c == ']' || c == ',' || c == '.');
}

int Lexer::get_nb() {
  int len = 0;
  int had_dot = 0;
  while (i + len < (int)_file.size() && isdigit(_file[i + len])) {
    len++;
    if (!had_dot && i + len < (int)_file.size() && _file[i + len] == '.') {
      had_dot = 1;
      len++;
    }
  }
  return (len);
}

std::string_view Lexer::get_token(t_tokenID id) {
  int len = 0;
  switch (id) {
  case NUMBER: {
    len = get_nb();
    std::string_view rep(&(_file[i]), len);
    i += len;
    col += len;
    return (rep);
    break;
  }
  case OPERATOR: {
    len = 1;
    if (i + len < (int)_file.size() && ft_isoperator(_file[i + len]))
      len++;
    std::string_view rep(&(_file[i]), len);
    i += len;
    col += len;
    return (rep);
    break;
  }
  case SEPARATOR: {
    len = 1;
    std::string_view rep(&(_file[i]), len);
    i += len;
    col += len;
    return (rep);
    break;
  }
  case WORD: {
    while (i + len < (int)_file.size() &&
           (std::isalnum(_file[i + len]) || _file[i + len] == '_')) {
      len++;
    }
    std::string_view rep(&(_file[i]), len);
    i += len;
    col += len;
    return (rep);
    break;
  }
  case STRING: {
    len++;
    while (i + len < (int)_file.size() && _file[i + len] != '"') {
      len++;
    }
    if (!_file[i + len]) {
      std::cerr << "String not ended\n";
      return ("");
    }
    len++;
    std::string_view rep(&(_file[i]), len);
    i += len;
    col += len;
    return (rep);
    break;
  }
  case EOI: {
    std::string_view rep(&(_file[i]), 1);
    i++;
    col++;
    return (rep);
  }
  default:
    i += len;
    col += len;
    std::string_view rep(&(_file[0]), 1);
    return (rep);
  }
  return ("");
}

uint8_t Lexer::handle_token() {
  t_token curr = {};
  curr.line = line;
  curr.col = col;
  if (isalpha(_file[i]) || _file[i] == '_') {
    curr.id = WORD;
  } else if (isdigit(_file[i])) {
    curr.id = NUMBER;
  } else if (_file[i] == '"') {
    curr.id = STRING;
  } else if (ft_isoperator(_file[i])) {
    curr.id = OPERATOR;
  } else if (ft_isseparator(_file[i])) {
    curr.id = SEPARATOR;
  } else if (_file[i] == ';') {
    curr.id = EOI;
  } else {
    std::cerr << "Not recognized character : `" << _file[i] << "` at " << line
              << ":" << col << ".\n";
    return (1);
  }
  curr.value = get_token(curr.id);
  try {
    _tokens.push_back(curr);
  } catch (const std::exception &e) {
    std::cerr << "Error : " << e.what() << ".\n";
    return 2;
  }
  return (0);
}

uint8_t Lexer::lex_file() {
  while (1) {
    if (i == (int)_file.size()) {
      break;
    } else if (_file[i] == '\n') {
      line++;
      col = 1;
      i++;
    } else if (iswspace(_file[i])) {
      col++;
      i++;
    } else if (_file[i] == '/' && _file[i + 1] == '/') {
      while (_file[i] && _file[i] != '\n')
        i++;
      if (!_file[i])
        break;
      col = 1;
      line++;
      i++;
    } else if (_file[i] == '/' && _file[i + 1] == '*') {
      while (_file[i] && !(_file[i] == '*' && _file[i + 1] == '/')) {
        if (_file[i] == '\n') {
          line++;
          col = 1;
        } else
          col++;
        i++;
      }
      if (_file[i]) {
        col += 2;
        i += 2;
      }
    } else if (handle_token() !=
               0) { // NOTE: This case is when there's an error
      return (1);
    }
  }
  t_token eofTok;
  eofTok.id = EOFF;
  eofTok.value = "";
  try {
    _tokens.push_back(eofTok);
  } catch (const std::exception &e) {
    std::cerr << "Error : " << e.what() << ".\n";
    return 2;
  }
  // Print();
  return (0);
}
