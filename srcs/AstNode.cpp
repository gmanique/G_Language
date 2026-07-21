#include "AstNode.hpp"

AstNode::AstNode() : _name("") {DEBUG(std::cout << "Creating Ast Node " << _name << "\n";);}
AstNode::AstNode(std::string name) : _name(name) {DEBUG(std::cout << "Creating Ast Node " << _name << "\n";);}
AstNode::AstNode(std::string name, t_cursor pos) : _name(name), _pos(pos) {DEBUG(std::cout << "Creating Ast Node " << _name << "\n";);}
AstNode::~AstNode() {
	for(size_t i = 0; i < this->_childrens.size(); i++) {
		delete this->_childrens[i];
	}
}

void		AstNode::setName(std::string Name) {this->_name = Name;}
std::string	&AstNode::getName() {return (this->_name);}
void		AstNode::setPos(t_cursor pos) {this->_pos = pos;}
t_cursor	&AstNode::getPos() {return (this->_pos);}
void		AstNode::push_back(AstNode *ast) {this->_childrens.push_back(ast);}
AstNode		*AstNode::operator[](const unsigned int index) {return (this->_childrens[index]);}


void	AstNode::addArgs(std::pair<std::string, t_tokenId> &arg) {
	this->_args.push_back(arg);
}


void AstNode::printRecursive(const std::string& prefix, bool isLast) const {
    std::cout << prefix;
    std::cout << (isLast ? "└── " : "├── ");
    std::cout << _name;
    if (!_args.empty()) {
		std::cout << " [";
        for (size_t i = 0; i < _args.size(); ++i) {
            std::cout << _args[i].first;
            if (i < _args.size() - 1) std::cout << ", ";
        }
        std::cout << "]";
    }
    std::cout << "\n";
    std::string newPrefix = prefix + (isLast ? "    " : "│   ");
    for (size_t i = 0; i < _childrens.size(); ++i) {
        bool childIsLast = (i == _childrens.size() - 1);
        if (_childrens[i] != NULL) {
			_childrens[i]->printRecursive(newPrefix, childIsLast);
		}
	}
}

void	AstNode::print() const {
	this->printRecursive("", true);
}
