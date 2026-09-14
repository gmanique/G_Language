#include "Logging.hpp"

// namespace {
// void log_impl(std::string_view level, std::string_view msg,
//               const std::source_location &loc) {
//   std::cout << std::format("[{}] {}:{} dans {}() : {}\n", level,
//                            loc.file_name(), loc.line(), loc.function_name(),
//                            msg);
// }
// } // namespace
//
// void log_debug(std::string_view msg, const std::source_location loc) {
//   log_impl("DEBUG", msg, loc);
// }
// void log_info(std::string_view msg, const std::source_location loc) {
//   log_impl("INFO", msg, loc);
// }
// void log_warning(std::string_view msg, const std::source_location loc) {
//   log_impl("WARNING", msg, loc);
// }
// void log_error(std::string_view msg, const std::source_location loc) {
//   log_impl("ERROR", msg, loc);
// }
