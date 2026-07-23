#ifndef SYNTAXANALYZER_HPP
# define SYNTAXANALYZER_HPP

# include "includes.hpp"

typedef enum e_type {
	UNKNOWN = 0,	// sert a quand je trouve un mot que je ne connais pas
	FUNCTION,		// nom de fonction
	TYPE,			// i16, structure, etc
	KEYWORD,		// if, return, while
	VARIABLE		// Les variables creees
}	t_type;


class SyntaxAnalyzer {
	private:
		AstNode										*_tree;
		std::unordered_map<std::string, t_type>		_existing_elems;

		uint8_t	is_correct_variable(AstNode *member, std::unordered_set<std::string> &already_existing, uint8_t *isPtr, std::string& varName);
		void	func_def_check_args(std::vector<std::pair<std::string, t_tokenId>> &args, std::string &func_name, std::unordered_map<std::string, t_type> &self_elems);

		void	analyze_var_definition(AstNode *tree);
		void	analyze_function(AstNode *tree);
		void	analyze_struct(AstNode *tree);
	public:
		SyntaxAnalyzer();
		SyntaxAnalyzer(AstNode *tree);	
		~SyntaxAnalyzer();
		DECLARE_EXCEPTION(UnknownID); 
		DECLARE_EXCEPTION(AlreadyDefined); 


		void	register_elem(const std::string &elem_name, t_type type);
		t_type	get_element_type(const std::string &name) const;
		void	analyze();

};

#endif
