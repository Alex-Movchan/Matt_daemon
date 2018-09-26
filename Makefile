
NAME = Matt_daemon

FLAG = -Wall -Wextra -Werror

G = clang++

SRC = Tintin_reporter.cpp \
      Daemon.cpp \
      main.cpp

OBG = $(SRC:.cpp=.o)

DELL = rm -f

all: $(NAME)

$(NAME): $(OBG)
	@$(G) $(FLAG) $(SRC) -o $(NAME)

%.o: %.cpp
	$(G) $(FLAG) -c $< -o $@

clean:
	$(DELL) $(OBG)

fclean: clean
	$(DELL) $(NAME)

re: fclean all
