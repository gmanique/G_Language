
#include "FileReader.hpp"
#include <fstream>

// #include "Logging.hpp"

std::optional<std::string>
FileReader::read_file(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
    return (std::nullopt);
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open())
    return (std::nullopt);

  const auto size = file.tellg();
  std::string buffer(size, '\0');

  file.seekg(0);
  if (!file.read(buffer.data(), size)) {
    return (std::nullopt);
  }

  return (buffer);
}
