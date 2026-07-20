#ifndef INCLUDES_HPP
# define INCLUDES_HPP

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <list>
# include <cstring>
# include <cerrno>
# include <cstdlib>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <dirent.h>
# include <signal.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <sstream>
# include <sys/mman.h>


# define DECLARE_EXCEPTION(ExceptionName) \
 	   class ExceptionName : public std::exception { \
    	public: \
        	virtual const char *what() const throw(); \
		}

# include "Token.hpp"
# include "AstNode.hpp"
# include "Parser.hpp"
# include "SyntaxAnalyzer.hpp"

/* Permet d'implementer le make debug */
# ifdef DEBUG_MODE
#  define DEBUG(cmd) do { cmd; } while (0)
# else
#  define DEBUG(cmd) ((void)0)
# endif


#endif
