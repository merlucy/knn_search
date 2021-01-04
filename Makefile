all : getResults makeIndex


getResults : getResults.o
	g++ -o getResults getResults.o
makeIndex : makeIndex.o
	g++ -o makeIndex makeIndex.o

getResults.o : getResults.cpp
	g++ -c getResults.cpp
makeIndex.o : makeIndex.cpp
	g++ -c makeIndex.cpp
clean : 
	rm *.o
