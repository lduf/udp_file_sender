group = DetecteurML
options = -O3 -Wall -c -g
all : server1

server1 : server1.o utils.o stack.o tcp_mechanism.o
	gcc -O3 utils.o server1.o stack.o tcp_mechanism.o -o ../bin/server1-$(group) 

server1.o : server1.c includes/server1.h
	gcc $(options) -I /includes server1.c

utils.o : utils.c includes/utils.h
	gcc $(options) utils.c -I /includes -o utils.o

stack.o : stack.c includes/stack.h
	gcc $(options) stack.c -I /includes -o stack.o

tcp_mechanism.o : tcp_mechanism.c includes/tcp_mechanism.h includes/utils.h includes/stack.h
	gcc $(options) tcp_mechanism.c -I /includes -o tcp_mechanism.o

clean :
	rm -f *.o
	rm -f ../bin/server*

render : server1
	../bin/server1-$(group) 4314

package :
	tar -czvf ../../$(group).tar.gz ../
