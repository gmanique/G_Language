/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:22 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/16 02:28:50 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"
#include <exception>
#include <iostream>
#include <unordered_map>

static const std::unordered_map<std::string_view, t_tokenID> keywords = {
    {"if", KW_IF},         {"else", KW_ELSE},     {"while", KW_WHILE},
    {"fun", KW_FUN},       {"struct", KW_STRUCT}, {"enum", KW_ENUM},
    {"return", KW_RETURN}, {"i8", KW_TYPE},       {"i16", KW_TYPE},
    {"i32", KW_TYPE},      {"i64", KW_TYPE},      {"u8", KW_TYPE},
    {"u16", KW_TYPE},      {"u32", KW_TYPE},      {"u64", KW_TYPE},
    {"string", KW_TYPE},   {"import", KW_IMPORT}, {"syscall", KW_SYSCALL}};

Lexer::Lexer(std::string &file) : _file(file), i(0), line(1), col(1) {}
Lexer::Lexer(std::string_view &file) : _file(file), i(0), line(1), col(1) {}

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
  case CHAR: {
    len = 1;
    if (i + len < (int)_file.size() && _file[i + len] == '\'') {
      std::cerr << "Empty character literal at " << line << ":" << col << "\n";
      return {};
    }
    if (i + len < (int)_file.size()) {
      if (_file[i + len] == '\\') {
        len++;
      }
      if (i + len < (int)_file.size()) {
        len++;
      }
    }
    if (i + len >= (int)_file.size() || _file[i + len] != '\'') {
      std::cerr << "Unterminated character literal at " << line << ":" << col
                << "\n";
      return {};
    }
    len++;
    std::string_view rep(&(_file[i]), len);
    i += len;
    col += len;
    return rep;
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

    if (i + len == (int)_file.size()) {
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
  } else if (_file[i] == '\'') {
    curr.id = CHAR;
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
  if (curr.id == WORD) {
    auto it = keywords.find(curr.value);
    if (it != keywords.end()) {
      curr.id = it->second;
    }
  }
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
    } else if (std::isspace(_file[i])) {
      col++;
      i++;
    } else if (_file[i] == '/' && _file[i + 1] == '/') {
      while (i < (int)_file.size() && _file[i] != '\n')
        i++;
      if (!_file[i])
        break;
      col = 1;
      line++;
      i++;
    } else if (_file[i] == '/' && _file[i + 1] == '*') {
      while (i < (int)_file.size() &&
             !(_file[i] == '*' && _file[i + 1] == '/')) {
        if (_file[i] == '\n') {
          line++;
          col = 1;
        } else
          col++;
        i++;
      }
      if (i < (int)_file.size()) {
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
  return (0);
}
