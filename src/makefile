group = DetecteurML
all : server1

server1 : server1.go
	go build -o ../bin/server1-$(group) server1.go
clean :
	rm -f *.o
	rm -f ../bin/server1

render : server1
	../bin/server1-$(group) 4314

package :
	tar -czvf ../../$(group).tar.gz ../