#ifndef ASTNODE_HPP
# define ASTNODE_HPP

#include "includes.hpp"

class AstNode {
	private:
		std::string										_name;
		std::vector<std::pair<std::string, t_tokenId>>	_args;
		t_cursor										_pos;
		std::vector<AstNode *>							_childrens;
	
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
		void			addArgs(std::pair<std::string, t_tokenId> &arg);
		size_t			get_nb_childs() {return (_childrens.size());}
		void			print() const;
		std::vector<std::pair<std::string, t_tokenId>>	&getArgs() {
			return (this->_args);
		}
};

#endif
