#include "Logging.hpp"
#include <iostream>

#include <source_location>
// #include <iostream>
#include <format>

int main(int ac, char **av) {
  (void)av;
  if (ac < 2)
    std::cout << "Welcome in the Glang Compiler version 0.0.1 !\n";
  if (ac > 2)
    std::cout << "I can only handle one file at once yet.\n";

  Logger::debug("Debug test");
  Logger::info("Info test");
  Logger::warning("Warning test");
  Logger::error("Error test");
  return (0);
}
