#ifndef PARSER_HPP
# define PARSER_HPP

#include "includes.hpp"

# define MAX_FILE_SIZE	65535

class AstNode;

class Parser {
	private:
        t_cursor    		_cursor;
        std::string 		_file;
        uint16_t    		_file_size;
		std::list<t_token>	tokens;
		AstNode				*Tree;

		void	readFile(int &fd);
		void	LexFile();
		void	handleToken(int &i);
		void	MakeTree();
	public:
		std::string	getFile();
		void	Parse(int fd);
		Parser();
		~Parser();
};

void	parse_block(AstNode	*newNode, std::list<t_token>::iterator &cursor);
void	parse_directive(AstNode *tree, std::list<t_token>::iterator &cursor);

#endif
