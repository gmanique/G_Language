cmake_minimum_required(VERSION 3.20)

project(glang
	VERSION 0.2.0
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
)

add_executable(glang ${SOURCES})

target_include_directories(glang PRIVATE includes)

if(MSVC)
	target_compile_options(glang PRIVATE /W4 /WX)
else()
	target_compile_options(glang PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

file(CREATE_LINK
	"${CMAKE_BINARY_DIR}/compile_commands.json"
	"${CMAKE_SOURCE_DIR}/compile_commands.json"
	SYMBOLIC
)
