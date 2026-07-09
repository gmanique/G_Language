#ifndef PARSER_HPP
# define PARSER_HPP

#include "includes.hpp"

/* Les token id pour le tokenizer */
# define EOFF            (uint8_t)0
# define OPEN_BLOCK     (uint8_t)1
# define CLOSE_BLOCK    (uint8_t)2
# define EOI            (uint8_t)3
# define WORD           (uint8_t)4
# define STRING         (uint8_t)5

# define MAX_FILE_SIZE	65535

typedef struct s_cursor {
    uint16_t    line;
    uint16_t    col;
}   t_cursor;

typedef struct s_token {
    uint8_t     id;
    std::string content;
    t_cursor    pos;    /*pour les messages d'erreurs eventuels*/
}   t_token;

typedef struct s_list {
    t_token         curr;
    struct s_list   *next;
}   t_list;

class Parser {
	private:
        t_cursor    	_cursor;
        std::string 	_file;
        uint16_t    	_file_size;
        t_list      	*tokens;
		

		void	readFile(int &fd);
		void	LexFile();
	public:
		std::string	getFile();
		void	Parse(int fd);
		Parser();
		~Parser();
};

#endif
