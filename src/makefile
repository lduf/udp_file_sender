all : server1

server1 : server1.o
	gcc -o ../bin/server1 server1.o

server1.o : server1.c
	gcc -c server1.c

clean :
	rm -f *.o
	rm -f ../bin/server1

render : server1
	../bin/server1 1000

package :
	tar -czvf ../../LUCASDUFOUR.tar.gz ../