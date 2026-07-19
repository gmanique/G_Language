#ifndef ASTNODE_HPP
# define ASTNODE_HPP

#include "includes.hpp"

/*
// Pour voir la structure, elle est definie dans Parser.hpp
typedef struct s_token {
    t_tokenId	id;
    std::string	content;
    t_cursor	pos;    // pour les messages d'erreurs eventuels
}   t_token;
*/


class AstNode {
	private:
		std::string					_name;
		std::vector<std::string>	_args;
		t_cursor					_pos;
		std::vector<AstNode *>		_childrens;
	
	
		void printRecursive(const std::string& prefix, bool isLast) const;
	public:
		AstNode();
		AstNode(std::string name);
		AstNode(std::string name, t_cursor pos);
		~AstNode();	

		void			setName(std::string Name);
		std::string		&getName();
		void			setPos(t_cursor pos);
		t_cursor		&getPos();
		void			push_back(AstNode *ast);
		AstNode			*operator[](const unsigned int index);
		void			addArgs(std::string &arg);

		void			print() const;
};

#endif
