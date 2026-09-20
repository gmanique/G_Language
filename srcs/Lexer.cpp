/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:22 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 21:32:58 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"
#include <cctype>
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

unsigned char Lexer::peekc(int offset) const {
  int idx = i + offset;
  if (idx < 0 || idx >= (int)_file.size())
    return ('\0');
  return (static_cast<unsigned char>(_file[idx]));
}

void Lexer::advance(int n) {
  for (int k = 0; k < n && i < (int)_file.size(); k++) {
    if (_file[i] == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
    i++;
  }
}

static uint8_t ft_isoperator(unsigned char c) {
  return (c == '+' || c == '-' || c == '&' || c == '|' || c == '*' ||
          c == '/' || c == '%' || c == '=' || c == '!' || c == '<' || c == '>');
}

static bool ft_isdoubleoperator(unsigned char a, unsigned char b) {
  static const char *const pairs[] = {"==", "!=", "<=", ">=", "&&", "||"};
  for (const char *op : pairs) {
    if (op[0] == a && op[1] == b)
      return (true);
  }
  return (false);
}

static uint8_t ft_isseparator(unsigned char c) {
  return (c == '(' || c == '{' || c == '[' || c == ')' || c == '}' ||
          c == ']' || c == ',' || c == '.');
}

int Lexer::get_nb() {
  int len = 0;
  while (std::isdigit(peekc(len)))
    len++;
  if (peekc(len) == '.' && std::isdigit(peekc(len + 1))) {
    len++;
    while (std::isdigit(peekc(len)))
      len++;
  }
  return (len);
}

std::string_view Lexer::get_token(t_tokenID id) {
  int len = 0;
  switch (id) {
  case NUMBER:
    len = get_nb();
    break;
  case OPERATOR:
    len = 1;
    if (ft_isdoubleoperator(peekc(0), peekc(1)))
      len = 2;
    break;
  case SEPARATOR:
  case EOI:
    len = 1;
    break;
  case CHAR: {
    len = 1;
    if (peekc(len) == '\'') {
      std::cerr << "Empty character literal at " << line << ":" << col << "\n";
      return {};
    }
    if (peekc(len) == '\\')
      len++;
    if (i + len >= (int)_file.size() || peekc(len) == '\n') {
      std::cerr << "Unterminated character literal at " << line << ":" << col
                << "\n";
      return {};
    }
    len++;
    if (peekc(len) != '\'') {
      std::cerr << "Unterminated character literal at " << line << ":" << col
                << "\n";
      return {};
    }
    len++;
    break;
  }
  case WORD:
    while (std::isalnum(peekc(len)) || peekc(len) == '_')
      len++;
    break;
  case STRING: {
    len = 1;
    while (i + len < (int)_file.size() && peekc(len) != '"') {
      if (peekc(len) == '\\' && i + len + 1 < (int)_file.size())
        len++;
      len++;
    }
    if (i + len >= (int)_file.size()) {
      std::cerr << "Unterminated string literal at " << line << ":" << col
                << "\n";
      return {};
    }
    len++;
    break;
  }
  default:
    return {};
  }
  std::string_view rep(_file.data() + i, len);
  advance(len);
  return (rep);
}

uint8_t Lexer::handle_token() {
  t_token curr = {};
  curr.line = line;
  curr.col = col;
  const unsigned char c = peekc();
  if (std::isalpha(c) || c == '_') {
    curr.id = WORD;
  } else if (std::isdigit(c)) {
    curr.id = NUMBER;
  } else if (c == '"') {
    curr.id = STRING;
  } else if (c == '\'') {
    curr.id = CHAR;
  } else if (ft_isoperator(c)) {
    curr.id = OPERATOR;
  } else if (ft_isseparator(c)) {
    curr.id = SEPARATOR;
  } else if (c == ';') {
    curr.id = EOI;
  } else {
    std::cerr << "Not recognized character : ";
    if (c >= 0x20 && c < 0x7F)
      std::cerr << "`" << static_cast<char>(c) << "`";
    else
      std::cerr << "byte 0x" << std::hex << static_cast<int>(c) << std::dec;
    std::cerr << " at " << line << ":" << col << ".\n";
    return (1);
  }
  curr.value = get_token(curr.id);
  if (curr.value.empty())
    return (1); // NOTE: get_token already printed the reason
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
  while (i < (int)_file.size()) {
    const unsigned char c = peekc();
    if (std::isspace(c)) {
      advance(1);
    } else if (c == '/' && peekc(1) == '/') {
      while (i < (int)_file.size() && peekc() != '\n')
        advance(1);
    } else if (c == '/' && peekc(1) == '*') {
      const int startLine = line;
      const int startCol = col;
      advance(2); // NOTE: skip the opening, so "/*/" is not a full comment
      while (i < (int)_file.size() && !(peekc() == '*' && peekc(1) == '/'))
        advance(1);
      if (i >= (int)_file.size()) {
        std::cerr << "Unterminated block comment starting at " << startLine
                  << ":" << startCol << ".\n";
        return (1);
      }
      advance(2);
    } else if (handle_token() !=
               0) { // NOTE: This case is when there's an error
      return (1);
    }
  }
  t_token eofTok = {};
  eofTok.id = EOFF;
  eofTok.value = "";
  eofTok.line = line; // NOTE: was left uninitialized -> garbage in errors
  eofTok.col = col;
  try {
    _tokens.push_back(eofTok);
  } catch (const std::exception &e) {
    std::cerr << "Error : " << e.what() << ".\n";
    return 2;
  }
  return (0);
}
