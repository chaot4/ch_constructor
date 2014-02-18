# set -O2 if not overwritten by env/make argument
CPPFLAGS ?= -O2 -g
LDFLAGS ?= -g

# add necessary flags
override LDLIBS += -lstdc++
override CXXFLAGS += -std=c++11
override CPPFLAGS += -Wall -fopenmp
override LDFLAGS += -Wl,--no-undefined -fopenmp

BIN = main
SRCS = $(wildcard **/*.cpp) $(wildcard **/*.h) $(wildcard *.cpp) $(wildcard *.h)
DEPENDFILE = .depend

all: $(BIN)

main: main.o nodes_and_edges.o unit_tests.o
	$(CC) $(LDFLAGS) -o main main.o nodes_and_edges.o unit_tests.o $(LDLIBS)

main.o: nodes_and_edges.o

dep: $(DEPENDFILE)

$(DEPENDFILE): $(SRCS)
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $^ > $(DEPENDFILE)

-include $(DEPENDFILE)

.PHONY: clean
clean:
	rm -rf $(BIN) main.o nodes_and_edges.o .depend
