#include "includes.hpp"

int main(int ac, char **av) {
	int	fd;
	Parser	parse;

	if (ac < 2)
		return (std::cerr << "No Argument given.\n", 1);
	if (ac > 2)
		return (std::cerr << "Too many arguments given.\n", 1);
	fd = open(av[1], O_RDONLY);
	if (fd == -1)
		return (std::cerr << "Incorrect file.\n", 1);
	try {
		parse.Parse(fd);
	} catch (const std::exception &e) {
		std::cerr << e.what();
	}
	return (0);
}
