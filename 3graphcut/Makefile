# Makefile for CS 140 homework 1

# This is a list of the source (.c) files you use
SRC = main.cpp graph.cpp

# This is the name of the executable file you will run; here, ./matrix_multiply
EXEC = GraphCut

# This gives the compiler flags. Uncomment one of the following two lines.
# This one is for performance: use max optimization, skip assertions.
#CFLAGS = -Wall -m64 -DBUILD_64 -O3 -DNDEBUG
CFLAGS = `pkg-config --cflags opencv`
# This one is for debugging: generate symbols and check assertions.
# CFLAGS = -Wall -m64 -DBUILD_64 -g -O0 -DDEBUG
LFLAGS = `pkg-config --libs opencv`
 



# YOU PROBABLY DO NOT HAVE TO CHANGE ANYTHING BELOW THIS LINE.
 
# This generates a list of object file names from the source file names
OBJ = $(addsuffix .o, $(basename $(SRC)))

# "make" makes the executable.
$(EXEC): $(OBJ)
	g++ $(LFLAGS) $(OBJ) -o $(EXEC)

# This says how to build an object (.o) file from a source (.c) file
%.o : %.cpp
	g++ $(CFLAGS) -c $< -o $@

# "make clean" deletes objects and executable
clean:
	rm -f $(EXEC) *.o 
