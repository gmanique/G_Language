#ifndef TOKEN_HPP
# define TOKEN_HPP

# include "includes.hpp"

/* Les token id pour le tokenizer */
enum t_tokenId : uint8_t {
	TOKEN_EOF,
	TOKEN_OPEN_BLOCK,		// {
	TOKEN_CLOSE_BLOCK,		// }
	TOKEN_OPEN_BRACKET,		// [
	TOKEN_CLOSE_BRACKET,	// ]
	TOKEN_OPEN_PAREN,		// (
	TOKEN_CLOSE_PAREN,		// )
	TOKEN_SEMICOLON,		// ;
	TOKEN_COMMA,			// ,
	TOKEN_DOT,				// .
	TOKEN_WORD,				// identifiants / mots-clés (let, fun, nom de variable, i8, u16, ...)
	TOKEN_STRING,			// "texte"
	TOKEN_NUMBER,			// 42
	TOKEN_ASSIGN,			// =, +=, -=, *=, /=, %=
	TOKEN_OPERATOR			// +, -, *, /, >, <, <=, >=, !=, ++, --, &&, &, |, || ...
};

typedef struct s_cursor {
    uint16_t    line;
    uint16_t    col;
}   t_cursor;

typedef struct s_token {
    t_tokenId	id;
    std::string	content;
    t_cursor	pos;    /*pour les messages d'erreurs eventuels*/
}   t_token;

#endif
