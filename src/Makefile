group = DetecteurML
all : server1 utils

server1 : server1.o utils.o
	gcc utils.o server1.o -o ../bin/server1-$(group) 

server1.o : server1.c 
	gcc -Wall -c -g server1.c

utils.o : utils.c includes/utils.h
	gcc -Wall -c -g utils.c -I /includes -o utils.o

clean :
	rm -f *.o
	rm -f ../bin/server*

render : server1
	../bin/server1-$(group) 4314

package :
	tar -czvf ../../$(group).tar.gz ../
