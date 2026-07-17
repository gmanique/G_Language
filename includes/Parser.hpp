#ifndef PARSER_HPP
# define PARSER_HPP

#include "includes.hpp"


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
	TOKEN_OPERATOR			// +, -, *, /, >, <, <=, >=, !=, ++, --, ...
};


# define MAX_FILE_SIZE	65535

typedef struct s_cursor {
    uint16_t    line;
    uint16_t    col;
}   t_cursor;

typedef struct s_token {
    t_tokenId	id;
    std::string	content;
    t_cursor	pos;    /*pour les messages d'erreurs eventuels*/
}   t_token;

class Parser {
	private:
        t_cursor    		_cursor;
        std::string 		_file;
        uint16_t    		_file_size;
		std::list<t_token>	tokens;

		void	readFile(int &fd);
		void	LexFile();
		void	handleToken(int &i);
	public:
		std::string	getFile();
		void	Parse(int fd);
		Parser();
		~Parser();
};

#endif
