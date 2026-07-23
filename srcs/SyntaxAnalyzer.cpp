#include "SyntaxAnalyzer.hpp"

static void	init_elements(std::unordered_map<std::string, t_type> &elems) {
	elems.insert({
    	{"i8", TYPE}, {"i16", TYPE}, {"i32", TYPE}, {"i64", TYPE},
    	{"u8", TYPE}, {"u16", TYPE}, {"u32", TYPE}, {"u64", TYPE},
    	{"string", TYPE}, {"void", TYPE},
    	{"write", FUNCTION}, {"open", FUNCTION}, {"read", FUNCTION},
    	{"return", KEYWORD}, {"if", KEYWORD}, {"while", KEYWORD},
		{"else", KEYWORD},
		{"sizeof", KEYWORD}, {"fun", KEYWORD}, {"struct", KEYWORD},
		{"enum", KEYWORD}
	});
}
SyntaxAnalyzer::SyntaxAnalyzer() : _tree(NULL) {
	init_elements(this->_existing_elems);
}
SyntaxAnalyzer::SyntaxAnalyzer(AstNode *tree) : _tree(tree) {
	init_elements(this->_existing_elems);
}
SyntaxAnalyzer::~SyntaxAnalyzer() {}
const char *SyntaxAnalyzer::AlreadyDefined::what() const throw() {
	return ("Already defined.");
}
const char *SyntaxAnalyzer::UnknownID::what() const throw() {
	return ("Unknown id element.");
}



void	SyntaxAnalyzer::register_elem(const std::string &elem_name, t_type type) 
{
	if (_existing_elems.contains(elem_name)) {
		throw AlreadyDefined();
	}
	_existing_elems[elem_name] = type; 	
}

t_type SyntaxAnalyzer::get_element_type(const std::string &name) const {
    return (_existing_elems.contains(name) ? _existing_elems.at(name) : UNKNOWN);
}

uint8_t	SyntaxAnalyzer::is_correct_variable(AstNode *member, std::unordered_set<std::string> &already_existing, uint8_t *isPtr, std::string &varName) {
	std::string type_name = member->getName();
	varName = type_name; /* pour check que ce soit bien un pointeur si la meme variable*/
	if (this->get_element_type(type_name) != TYPE) {
		std::cerr << "Error at " << member->getPos().line << ":" << member->getPos().col << " - ";
		throw std::runtime_error("Unknown type '" + type_name + "'\n");
	}
    std::vector<std::pair<std::string, t_tokenId>> &args = member->getArgs();
	size_t i = 0;
	while(i < args.size()
		&& (args[i].second == TOKEN_OPEN_BRACKET
			|| args[i].first == "*"
			//|| args[i].first == "&" // Je sais pas si je gere les references, ca a l'air chiant
			)) {
		if (args[i].second == TOKEN_OPEN_BRACKET) {
			i++;
			if (i < args.size() && args[i].second == TOKEN_NUMBER)
				i++;
			if (i >= args.size() || args[i].second != TOKEN_CLOSE_BRACKET)
				throw std::runtime_error("Ya un probleme avec tes `[` `]` .\n");
		}
		else if (args[i].first == "*" && isPtr)
			*isPtr = 1;
		i++;
	}
	if (i == args.size())
		throw std::runtime_error("Variable declaration doesn't have name.\n");
	if (args[i].second != TOKEN_WORD)
		throw std::runtime_error("Variable declaration doesn't have a correct name : `" + args[i].first + "`.\n");
	std::string	var_name = args[i].first;
	
	i++;
	if (i != args.size()) {
		throw std::runtime_error("Don't add anything after the variable name please.\n");
	}
	if (already_existing.contains(var_name))
		throw std::runtime_error("You cannot have multiple variables with the same name : `" + var_name + "`.\n");
	already_existing.insert(var_name);
	return (1);
}



void	SyntaxAnalyzer::analyze_struct(AstNode *tree) {
	std::vector<std::pair<std::string, t_tokenId>> &args = tree->getArgs();
	if (args.size() != 1)
		throw std::runtime_error("Syntax Error: 'struct' expects exactly 1 argument (struct name)\n");
	if (args[0].second != TOKEN_WORD)
		throw std::runtime_error("Syntax Error: 'struct' expects a word as argument (struct name)\n");
	std::string struct_name = args[0].first;
	if (this->get_element_type(struct_name) != UNKNOWN)
		throw AlreadyDefined();
	this->register_elem(struct_name, TYPE);
	std::unordered_set<std::string> self_existing_elems; // noms de variable quil connait deja
	for(size_t i = 0; i < tree->get_nb_childs(); i++) {
		AstNode *member = (*tree)[i];
		uint8_t	isPtr = 0;
		std::string	name;
		if (!this->is_correct_variable(member, self_existing_elems, &isPtr, name)) {
			std::cerr << "Test\n";
		}
		if (!isPtr && name == struct_name)
			throw std::runtime_error("Need the variable to be a pointer if used in itself's definition.\n");
	}
}





void	SyntaxAnalyzer::func_def_check_args(std::vector<std::pair<std::string, t_tokenId>> &args, std::string &func_name, std::unordered_map<std::string, t_type> &self_elems) {
	size_t	nb_elems = args.size();
	(void)self_elems;
	/* ajouter chaque variable a self_elems */
	if (args[1].second != TOKEN_OPEN_PAREN)
		throw std::runtime_error("Syntax Error: 'fun' expects parenthesis after function name\n");
	size_t i = 2;
	std::unordered_set<std::string>	func_args;
	while (i < nb_elems && args[i].second != TOKEN_CLOSE_PAREN) {
		std::vector<std::pair<std::string, t_tokenId>> argument;
		if (args[i].second == TOKEN_COMMA) {
			if (i == 2)
				throw std::runtime_error("Syntax Error : Missing argument ?\n");
			i++;
		}
		while (i < nb_elems && args[i].second != TOKEN_CLOSE_PAREN
				&& args[i].second != TOKEN_COMMA) {
			argument.push_back(args[i]);
			i++;
		}
		if (argument.size() == 0)
			throw std::runtime_error("Syntax Error : Missing argument ?\n");
		if (this->get_element_type(argument[0].first) != TYPE) {
            throw std::runtime_error("Unknown type '" + argument[0].first + "' in func '" + func_name + "'\n");
		}
		uint8_t	has_name = 0;
		size_t	name_index = 0;
		for (size_t j = 1; j < argument.size(); j++) {
			if (argument[j].second == TOKEN_WORD) {
				name_index = j;
				has_name++;
			}
		}
		if (has_name == 0)
			throw std::runtime_error("Syntax Error : Argument without a name ?\n");
		if (has_name > 1)
			throw std::runtime_error("Syntax Error : Argument cannot have multiple names.\n");
		if (func_args.contains(argument[name_index].first)) {
			throw std::runtime_error("Syntax Error : Argument cannot have same names.\n");
		}
		func_args.insert(argument[name_index].first);
	}
	if (i == args.size())
		throw std::runtime_error("Syntax Error : Need closing parenthesis at the end of function declaration.\n");
	if (this->get_element_type(args[args.size()-1].first) != TYPE) {
		throw std::runtime_error("Syntax Error : Missing or incorrect return type at the end of function declaration.\n");
	}
}

void	SyntaxAnalyzer::analyze_function(AstNode *tree) {
	std::vector<std::pair<std::string, t_tokenId>> &args = tree->getArgs();
	if (args.size() < 1)
		throw std::runtime_error("Syntax Error: 'fun' expects arguments");
	if (args[0].second != TOKEN_WORD)
		throw std::runtime_error("Syntax Error: 'fun' expects a word as argument (function name)");
	std::string	func_name = args[0].first;
	if (this->get_element_type(func_name) != UNKNOWN)
		throw AlreadyDefined();
	
	std::unordered_map<std::string, t_type> self_elems = this->_existing_elems;
	this->func_def_check_args(args, func_name, self_elems);
	for(size_t i = 0; i < tree->get_nb_childs(); i++) {
		AstNode *curr = (*tree)[i];
		t_type first_elem = this->get_element_type(curr->getName());
		if (first_elem == TYPE) {
			
			;// gerer variable
		}
		else if (first_elem == KEYWORD) {
			;
		}
		// check le body de la fonction
	}
}

void	SyntaxAnalyzer::analyze() {
	for(size_t i = 0; i < this->_tree->get_nb_childs(); i++) {
		AstNode	*child = (*this->_tree)[i];
		std::string &name = child->getName();
		if (name == "struct") {
			this->analyze_struct(child);
		} else if (name == "fun") {
			this->analyze_function(child);
		} else {
			std::cerr << name << " : \n";
			throw UnknownID();
		}
	}
}
