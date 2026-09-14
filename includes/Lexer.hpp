/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:43 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/15 01:09:44 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
#define LEXER_HPP

#include <cstdint>
#include <list>
#include <string>
#include <string_view>

typedef enum e_tokenID { EOFF, NUMBER, OPERATOR, SEPARATOR, WORD } t_tokenID;

typedef struct s_token {
  t_tokenID id;
  std::string_view value;
} t_token;

class Lexer {
private:
  std::list<t_token> _tokens;
  std::string_view _file;

public:
  Lexer(std::string &file);
  Lexer(std::string_view &file);
  uint8_t lex_file();
};

#endif
