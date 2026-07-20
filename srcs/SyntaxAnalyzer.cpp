#include "SyntaxAnalyzer.hpp"


SyntaxAnalyzer::SyntaxAnalyzer() : _tree(NULL) {}
SyntaxAnalyzer::SyntaxAnalyzer(AstNode *tree) : _tree(tree) {}
SyntaxAnalyzer::~SyntaxAnalyzer() {}

const char *SyntaxAnalyzer::UnknownID::what() const throw() {
	return ("Unknown id element.");
}



void	SyntaxAnalyzer::analyze() {

}
