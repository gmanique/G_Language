// #include "Logging.hpp"
#include <FileReader.hpp>
// #include <format>
#include <iostream>

int main(int ac, char **av) {
  if (ac < 2)
    return (std::cout << "Welcome in the Glang Compiler version 0.0.1 !\n", 0);
  if (ac > 2)
    return (std::cout << "I can only handle one file at once yet.\n", 0);

  std::string path(av[1]);
  FileReader a;
  std::optional<std::string> tmp = a.read_file(path);
  if (tmp.has_value()) {
    std::cout << tmp.value();
  }
  return (0);
}
