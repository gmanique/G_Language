#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include <filesystem>
#include <optional>
#include <string>

class FileReader {
public:
  std::optional<std::string> read_file(const std::filesystem::path &path);

  // private:
};

#endif
