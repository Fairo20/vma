CC=g++
CFLAGS=-I.
DEPS = bag/bag.hpp
OBJ = main.cpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -O3

speedymake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -O3
