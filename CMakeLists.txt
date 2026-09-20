cmake_minimum_required(VERSION 3.20)

project(glang
  VERSION 0.3.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
  srcs/main.cpp
  srcs/Compiler.cpp
  srcs/FileReader.cpp
  srcs/Lexer.cpp
  srcs/Lexer_debug.cpp
  srcs/Parser.cpp
  srcs/Parser_debug.cpp
	srcs/Scope.cpp
)

add_executable(glang ${SOURCES})

target_include_directories(glang PRIVATE includes)

option(GLANG_WERROR   "Treat warnings as errors" ON)
option(GLANG_SANITIZE "Build with AddressSanitizer + UBSan (GCC/Clang)" OFF)

if(MSVC)
  target_compile_options(glang PRIVATE /W4)
  if(GLANG_WERROR)
    target_compile_options(glang PRIVATE /WX)
  endif()
else()
  target_compile_options(glang PRIVATE -Wall -Wextra -Wpedantic)
  if(GLANG_WERROR)
    target_compile_options(glang PRIVATE -Werror)
  endif()
  if(GLANG_SANITIZE)
    target_compile_options(glang PRIVATE -g -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(glang PRIVATE -fsanitize=address,undefined)
  endif()
endif()

file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)
