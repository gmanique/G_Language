#include "Parser.hpp"

Parser::Parser() {
	DEBUG(std::cout << "Initializing parser.\n";);
}

Parser::~Parser() {
	DEBUG(std::cout << "Destroying parser.\n";);
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

void	Parser::LexFile() {
	int	i = 0;
	this->_cursor.line = 1;
	this->_cursor.col = 0;
	while(i < this->_file_size) {
		if (this->_file[i] == '\n') {
			this->_cursor.line++;
			this->_cursor.col = 0;
			i++;
			continue;
		} else if (std::isspace(this->_file[i])) {
			this->_cursor.col++;
			i++;
			continue;
		} else if (this->_file[i] == '#') {
			while(i < this->_file_size && this->_file[i] != '\n')
				i++;
			continue;
		} else if (1) /*faire la suite*/
		{i++;}
	}
}

void	Parser::Parse(int fd) {
	this->readFile(fd);
	if (fd == -1)
		return ;
	this->LexFile();
}

