all: do

do: main.o nodeDht.o utill.o init.o port.o functions.o 
	g++ main.o init.o functions.o port.o nodeDht.o utill.o -o chorddht -lcrypto -lpthread
	
main.o: main.cpp
		g++ -std=c++11 -c main.cpp
		
init.o: init.cpp
		g++ -std=c++11 -c init.cpp

port.o: port.cpp
		g++ -std=c++11 -c port.cpp
		
functions.o: functions.cpp
		     g++ -std=c++11 -c functions.cpp
			 
nodeDht.o: nodeDht.cpp
		   g++ -std=c++11 -c nodeDHT.cpp
				   
utill.o: utill.cpp
		 g++ -std=c++11 -c utill.cpp			


	
