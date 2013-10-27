CC = g++
CPPFLAGS = -Wall -g -std=c++11
LFLAGS =
BIN = main
OBJ = 
SRCS = $(wildcard **/*.cpp) $(wildcard **/*.h) $(wildcard *.cpp) $(wildcard *.h)
DEPENDFILE = .depend

all: $(BIN)

$(BIN): $(OBJ) main.o
	$(CC) $(CPPFLAGS) $(LFLAGS) $(OBJ) main.cpp -o $(BIN)

dep: $(DEPENDFILE)

$(DEPENDFILE): $(SRCS)
	$(CC) -MM $(CPPFLAGS) $^ > $(DEPENDFILE)

-include $(DEPENDFILE)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(BIN) main.o
