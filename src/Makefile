CC=g++
CFLAGS=-I.
DEPS = $(wildcard *.hpp)
OBJ = main.cpp

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -O3

speedymake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -O3
