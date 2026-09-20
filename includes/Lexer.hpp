/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:43 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/20 21:17:09 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <cstdint>
#include <list>
#include <string>
#include <string_view>

typedef enum e_tokenID {
  EOFF,
  NUMBER,
  OPERATOR,
  SEPARATOR,
  WORD,
  CHAR,
  EOI,
  STRING,
  KW_IF,
  KW_ELSE,
  KW_WHILE,
  KW_RETURN,
  KW_STRUCT,
  KW_FUN,
  KW_ENUM,
  KW_TYPE,
  KW_IMPORT,
  KW_SYSCALL
} t_tokenID;

typedef struct s_token {
  t_tokenID id;
  std::string_view value;
  uint32_t line;
  uint32_t col;
} t_token;

class Lexer {
private:
  std::string_view _file;
  int i;
  int line;
  int col;
  std::list<t_token> _tokens;

  uint8_t handle_token();
  int get_nb();
  std::string_view get_token(t_tokenID id);

  // NOTE: Safe accessor: returns '\0' when out of range instead of reading
  // past the end of the buffer.
  unsigned char peekc(int offset = 0) const;
  // NOTE: Single place where i / line / col move forward, so line and col
  // stay correct whatever we are consuming (token, comment, string, ...).
  void advance(int n);

public:
  Lexer(std::string &file);
  Lexer(std::string_view &file);
  uint8_t lex_file();

  const std::list<t_token> &get_tokens() const { return _tokens; }

  void Print();
};

#endif
