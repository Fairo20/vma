CC=g++
CFLAGS=-I.
DEPS = bag/bag.hpp
OBJ = main.cpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

speedymake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
