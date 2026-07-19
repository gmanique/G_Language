#include "Parser.hpp"

Parser::Parser() {
	DEBUG(std::cout << "Initializing parser.\n";);
}

Parser::~Parser() {
	DEBUG(std::cout << "Destroying parser.\n";);
	if (this->Tree)
		delete this->Tree;
}

std::string	Parser::getFile() {
	return (this->_file);
}

void	Parser::readFile(int &fd) {
	struct stat st;
	char *file;
	if (fstat(fd, &st) < 0) {
		close(fd);
		fd = -1;
		return ;
	}
	if (((unsigned long)(st.st_size)) > MAX_FILE_SIZE) {
		close(fd);
		fd = -1;
		return ;
	}
    this->_file_size = (unsigned long)st.st_size;
    if (this->_file_size == 0) {
 		close(fd);
		fd = -1;
		return ;
	}
	file = (char *)mmap(NULL, (unsigned long)this->_file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (file == MAP_FAILED) {
		fd = -1;
		return ;
	}
	this->_file = file;
	munmap(file, this->_file_size);
}

inline bool is_operator(unsigned char c) {
    static const std::array<bool, 256> lut = []() {
        std::array<bool, 256> table = { false };
        table['+'] = true; table['-'] = true; table['*'] = true; 
        table['/'] = true; table['%'] = true; table['='] = true;
        table['<'] = true; table['>'] = true; table['!'] = true;
        table['&'] = true; table['|'] = true; table['^'] = true;
        return table;
    }();
    return lut[c];
}

static void	handle_operator(t_token &curr, int &i, int &len, std::string &file, t_cursor &pos) {
	if (file[i+1] == '=' &&
		(file[i] == '+' || file[i] == '-' || file[i] == '*'
		|| file[i] == '%' || file[i] == '/')) {
		curr.id = TOKEN_ASSIGN;
		len = 2;
	}
	else if (file[i] == '=' && !is_operator(file[i+1])) { 
		curr.id = TOKEN_ASSIGN;
		len = 1;
	}
	else if (is_operator(file[i+1])) {
		curr.id = TOKEN_OPERATOR;
		len = 2;
	}
	else {
		curr.id = TOKEN_OPERATOR;
		len = 1;
	}
	pos.col += len;
}

static void	handle_digit(t_token &curr, int &i, int &len, std::string &file, t_cursor &pos, const uint16_t &file_size) {
	while (i + len < file_size && std::isdigit(static_cast<unsigned char>(file[i+len]))) {
		len++;
	}
	curr.id = TOKEN_NUMBER;
	pos.col += len;
}

static void	handle_alpha(t_token &curr, int &i, int &len, std::string &file, t_cursor &pos, const uint16_t &file_size) {
	while (i + len < file_size && (file[i+len] == '_' || std::isalpha(static_cast<unsigned char>(file[i+len])) || std::isdigit(static_cast<unsigned char>(file[i+len])))) {
		len++;
	}
	curr.id = TOKEN_WORD;
	pos.col += len;
}

static void	handle_string(t_token &curr, int &i, int &len, std::string &file, t_cursor &pos, const uint16_t &file_size) {
	len++;
	pos.col++;
	while(i+len < file_size && file[i+len] != '"') {
		pos.col++;
		if (file[i+len] == '\n') {
			pos.col = 0;
			pos.line++;
		}
		len++;
	}
	if (i+len == file_size) {
		throw std::runtime_error("Not closed quote");
	}
	len++;
	curr.id = TOKEN_STRING;
}

void	Parser::handleToken(int &i) {
	t_token curr;
	int	len = 0;
	curr.pos = this->_cursor;
	if (this->_file[i] == '_' || std::isalpha(static_cast<unsigned char>(this->_file[i]))) {
		handle_alpha(curr, i, len, this->_file, this->_cursor, this->_file_size);
	} else if (std::isdigit(static_cast<unsigned char>(this->_file[i]))) {
		handle_digit(curr, i, len, this->_file, this->_cursor, this->_file_size);
	} else if (this->_file[i] == ';') {
		curr.id = TOKEN_SEMICOLON;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == ',') {
		curr.id = TOKEN_COMMA;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == '.') {
		curr.id = TOKEN_DOT;
		len = 1;
		this->_cursor.col++;
	}
	else if (this->_file[i] == ']') {
		curr.id = TOKEN_CLOSE_BRACKET;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == '[') {
		curr.id = TOKEN_OPEN_BRACKET;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == ')') {
		curr.id = TOKEN_CLOSE_PAREN;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == '(') {
		curr.id = TOKEN_OPEN_PAREN;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == '}') {
		curr.id = TOKEN_CLOSE_BLOCK;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == '{') {
		curr.id = TOKEN_OPEN_BLOCK;
		len = 1;
		this->_cursor.col++;
	} else if (this->_file[i] == '"') {
		handle_string(curr, i, len, this->_file, this->_cursor, this->_file_size);
	} else if (is_operator(static_cast<unsigned char>(this->_file[i]))) {
		handle_operator(curr, i, len, this->_file, this->_cursor);
	} else {
		std::cout << "Not handled : `" << this->_file[i] << "`\n";
		i++;
		len = 1;
	}
	curr.content = this->_file.substr(i, len);
	DEBUG(std::cout << "Token : " << this->_cursor.line << ":" << this->_cursor.col <<  " : `" << curr.content << "`\n";);
	i += len;
	try { this->tokens.push_back(curr); } catch (const std::exception &e) { std::cerr << e.what() << "\n"; throw; }
}

void	Parser::LexFile() {
	int	i = 0;
	this->_cursor.line = 1;
	this->_cursor.col = 0;
	while(i < this->_file_size) {

			/*whitespaces */
		if (this->_file[i] == '\n') {
			this->_cursor.line++;
			this->_cursor.col = 0;
			i++;
			continue;
		} else if (std::isspace(this->_file[i])) {
			this->_cursor.col++;
			i++;
			continue;
		
			/* commentaires */
		} else if (this->_file[i] == '/' && this->_file[i+1] == '/') {
			while(i < this->_file_size && this->_file[i] != '\n') {
				this->_cursor.col++;
				i++;
			}
			continue;
		} else if (this->_file[i] == '/' && this->_file[i+1] == '*') {
			i += 2;
			this->_cursor.col+=2;
			while(i < this->_file_size) {
				if (this->_file[i] == '*' && this->_file[i+1] == '/') {
					i += 2;
					this->_cursor.col += 2;
					break;
				}
				this->_cursor.col++;
				if (this->_file[i] == '\n') {
					this->_cursor.line++;
					this->_cursor.col = 0;
				}
				i++;
			}
			continue;

			/* tokens */
		} else {
			this->handleToken(i);
		}
	}
	t_token eoff = {TOKEN_EOF, "", this->_cursor};
	try { this->tokens.push_back(eoff); } catch (const std::exception &e) { std::cerr << e.what() << "\n"; throw; }
}


void	parse_block(AstNode	*newNode, std::list<t_token>::iterator &cursor) {
	cursor++;
	while(cursor->id != TOKEN_CLOSE_BLOCK && cursor->id != TOKEN_EOF) {
		if (cursor->id == TOKEN_WORD) {
			parse_directive(newNode, cursor);
		} else if (cursor->id == TOKEN_OPEN_BLOCK) {
			AstNode	*newNode2 = new AstNode("", cursor->pos);
			parse_block(newNode2, cursor);
			newNode->push_back(newNode2);
		} else {
			newNode->addArgs(cursor->content);
			/*int	line = cursor->pos.line;
			int	col = cursor->pos.col;
			std::cerr << "What the hell is `" << cursor->content << "` doing here at " << line << ":" << col << " ??? Mais version block !\n"; 
			// mettre meilleur msg d'erreur
			*/
			cursor++;	
		}
	}
	if (cursor->id == TOKEN_CLOSE_BLOCK) {
		cursor++;
		if (cursor->id == TOKEN_SEMICOLON)
			cursor++;
	}
	else
		std::cerr << "Bloc non ferme.\n";
}

void	parse_directive(AstNode *tree, std::list<t_token>::iterator &cursor) {
	AstNode	*newNode = new AstNode(cursor->content, cursor->pos);
	cursor++;
	while(cursor->id != TOKEN_SEMICOLON && cursor->id != TOKEN_EOF
			&& cursor->id != TOKEN_OPEN_BLOCK) {
		newNode->addArgs(cursor->content);
		cursor++;
	}
	switch(cursor->id) {
		case TOKEN_SEMICOLON: cursor++;break;
		case TOKEN_EOF:	delete(newNode); throw std::runtime_error("Ya un probleme la, une instruction sans `;` a la fin.\n");
		case TOKEN_OPEN_BLOCK:	parse_block(newNode, cursor); break; 
		default:break;
	}
	tree->push_back(newNode);
}


void	Parser::MakeTree() {
	this->Tree = new AstNode("main_tree");
	std::list<t_token>::iterator cursor = this->tokens.begin();
	try {
		
		while(cursor != this->tokens.end() && cursor->id != TOKEN_EOF) {
			if (cursor->id == TOKEN_WORD) {
				DEBUG(std::cout << "Handling :" << cursor->content << std::endl;);
				parse_directive(this->Tree, cursor);
			} else if (cursor->id == TOKEN_OPEN_BLOCK) {
				parse_block(this->Tree, cursor);
			} else { //error management block
				int	line = cursor->pos.line;
				int	col = cursor->pos.col;
				std::cerr << "What the hell is `" << cursor->content << "` doing here at " << line << ":" << col << " ???\n"; 
				// mettre meilleur msg d'erreur
				cursor++;
			}
		}

	}
	catch (const std::exception &e)
	{
		delete(this->Tree);
		this->Tree = NULL;
		throw;
	}
	DEBUG(this->Tree->print(););
}

void	Parser::Parse(int fd) {
	this->readFile(fd);
	if (fd == -1)
		return ;
	this->LexFile();
	this->MakeTree();
}

