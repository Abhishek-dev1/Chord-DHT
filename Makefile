CFLAGS = -std=c++11 -I./headerFile -I./nodeClass -I./utility/Function -I./utility/initialization -I./utility/nodeDhtUtill -I./utility/portFunction -I./utility
LDFLAGS = -lcrypto -lpthread

all: do

do: main.o nodeDht.o utill.o init.o port.o functions.o 
	g++ main.o init.o functions.o port.o nodeDht.o utill.o -o chorddht $(LDFLAGS)
	
main.o: main.cpp
	g++ $(CFLAGS) -c main.cpp
		
init.o: utility/initialization/init.cpp
	g++ $(CFLAGS) -c utility/initialization/init.cpp

port.o: utility/portFunction/port.cpp
	g++ $(CFLAGS) -c utility/portFunction/port.cpp
		
functions.o: utility/Function/functions.cpp
	g++ $(CFLAGS) -c utility/Function/functions.cpp
			 
nodeDht.o: nodeClass/nodeDht.cpp
	g++ $(CFLAGS) -c nodeClass/nodeDht.cpp
				   
utill.o: utility/nodeDhtUtill/utill.cpp
	g++ $(CFLAGS) -c utility/nodeDhtUtill/utill.cpp

clean:
	rm -f *.o chorddht			


	
