/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmanique <gmanique@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/15 01:09:34 by gmanique          #+#    #+#             */
/*   Updated: 2026/09/15 01:09:39 by gmanique         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <format>
#include <iostream>
#include <source_location>
#include <string_view>

enum class LogLevel { Debug, Info, Warning, Error };

constexpr std::string_view to_string(LogLevel level) noexcept {
  switch (level) {
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    return "INFO";
  case LogLevel::Warning:
    return "WARNING";
  case LogLevel::Error:
    return "ERROR";
  }
  return "UNKNOWN";
}

class Logger {
private:
  template <typename... Args>
  static void log_impl(LogLevel level, const std::source_location &loc,
                       std::format_string<Args...> fmt, Args &&...args) {
    std::cout << std::format("[{}] {}:{} dans {} : {}\n", to_string(level),
                             loc.file_name(), loc.line(), loc.function_name(),
                             std::format(fmt, std::forward<Args>(args)...));
  }

public:
  template <typename... Args>
  static void
  debug(std::format_string<Args...> fmt, Args &&...args,
        const std::source_location loc = std::source_location::current()) {
    log_impl(LogLevel::Debug, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void
  info(std::format_string<Args...> fmt, Args &&...args,
       const std::source_location loc = std::source_location::current()) {
    log_impl(LogLevel::Info, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void
  warning(std::format_string<Args...> fmt, Args &&...args,
          const std::source_location loc = std::source_location::current()) {
    log_impl(LogLevel::Warning, loc, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void
  error(std::format_string<Args...> fmt, Args &&...args,
        const std::source_location loc = std::source_location::current()) {
    log_impl(LogLevel::Error, loc, fmt, std::forward<Args>(args)...);
  }
};

#endif
