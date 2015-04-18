CC = g++
PROG = middleware client

DEPS = Header.h
CPLUSFLAGS = -std=c++0x -c -ggdb -O2 -Wall

MIDDLEWARE_SRC = Middleware.cpp ErrorFunctions.cpp
MIDDLEWARE_OBJS = $(MIDDLEWARE_SRC:.c=.o)

CLIENT_SRC = Client.cpp ErrorFunctions.cpp
CLIENT_OBJS = $(CLIENT_SRC:.cpp=.o)

all: $(PROG)

middleware: $(MIDDLEWARE_SRC)
	${CC} $(CPLUSFLAGS) -c -o Middleware.o Middleware.cpp
	${CC} $(CPLUSFLAGS) -c -o ErrorFunctions.o ErrorFunctions.cpp
	${CC} -o $@ $(MIDDLEWARE_OBJS)

client: $(CLIENT_SRC)
	${CC} $(CPLUSFLAGS) -c -o Client.o Client.cpp
	${CC} $(CPLUSFLAGS) -c -o ErrorFunctions.o ErrorFunctions.cpp
	${CC} -o $@ $(CLIENT_OBJS)

backup:
	rm -f awbrenn-hw3.tar *.o *.out client *~
	tar -cvzf awbrenn_akshitg.tar.gz *.cpp *.h README Makefile

server:
	./middleware

user:
	./client http://127.0.0.1/test/test.html

clean:
	rm -f $(PROG) *.o *.out client *~
