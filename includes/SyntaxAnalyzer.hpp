#ifndef SYNTAXANALYZER_HPP
# define SYNTAXANALYZER_HPP

# include "includes.hpp"

class SyntaxAnalyzer {
	private:
		AstNode	*_tree;

	public:
		SyntaxAnalyzer();
		SyntaxAnalyzer(AstNode *tree);	
		~SyntaxAnalyzer();
		DECLARE_EXCEPTION(UnknownID); 

		void	analyze();
};

#endif
