CXX 		:=	g++
CXX_FLAGS	:=	-c -Wall
LIB			:=	-pthread

all: main

main: main.o
	$(CXX) main.o $(LIB) -o main

main.o: main.cpp
	$(CXX) $(CXX_FLAGS) main.cpp

clean:
	rm -f *.o main