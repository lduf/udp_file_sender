group = DetecteurML
options = -O3 -Wall -c -g
all : server1 server2 server3

server1 : server1.o utils.o stack.o tcp_mechanism.o
	gcc -O3 utils.o server1.o stack.o tcp_mechanism.o -o ../bin/server1-$(group) 

server1.o : server1.c includes/server1.h
	gcc $(options) -I /includes server1.c

server2 : server2.o utils.o stack.o tcp_mechanism2.o
	gcc -O3 utils.o server2.o stack.o tcp_mechanism2.o -o ../bin/server2-$(group) 

server2.o : server2.c includes/server2.h
	gcc $(options) -I /includes server2.c

server3 : server3.o utils.o stack.o tcp_mechanism.o
	gcc -O3 utils.o server3.o stack.o tcp_mechanism.o -o ../bin/server3-$(group) 

server3.o : server3.c includes/server3.h
	gcc $(options) -I /includes server3.c

utils.o : utils.c includes/utils.h
	gcc $(options) utils.c -I /includes -o utils.o

stack.o : stack.c includes/stack.h
	gcc $(options) stack.c -I /includes -o stack.o

tcp_mechanism.o : tcp_mechanism.c includes/tcp_mechanism.h includes/utils.h includes/stack.h
	gcc $(options) tcp_mechanism.c -I /includes -o tcp_mechanism.o

tcp_mechanism2.o : tcp_mechanism2.c includes/tcp_mechanism2.h includes/utils.h includes/stack.h
	gcc $(options) tcp_mechanism2.c -I /includes -o tcp_mechanism2.o

clean :
	rm -f *.o
	rm -f ../bin/server*

render : server1
	../bin/server1-$(group) 4314

package :
	tar -czvf ../../$(group).tar.gz ../
