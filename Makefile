NAME = codexion
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pthread -g3 -O0
INCLUDES = -Iheaders
SRCS = 	src/heap/heap_create.c \
		src/heap/heap_utils.c \
		src/thread/init.c \
		src/thread/log.c \
		src/thread/operations.c \
		src/thread/run_utils.c \
		src/thread/run.c \
		src/thread/time.c \
		src/thread/utils.c \
		src/free.c \
		src/main.c \
		src/parsing.c

OBJS = ${SRCS:.c=.o}

all: ${NAME}

${NAME}: ${OBJS}
	$(CC) $(OBJS) -o $(NAME)
%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@
clean:
	rm -f ${OBJS}
fclean: clean
	rm -f ${NAME}
re: fclean all

.PHONY: all clean fclean re