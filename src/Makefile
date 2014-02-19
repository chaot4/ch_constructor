# set -O2 if not overwritten by env/make argument
CPPFLAGS ?= -O2 -g
LDFLAGS ?= -g

# add necessary flags
override LDLIBS += -lstdc++
override CXXFLAGS += -std=c++11
override CPPFLAGS += -Wall -fopenmp
override LDFLAGS += -Wl,--no-undefined -fopenmp

BIN = ch_constructor run_tests
SRCS = $(wildcard **/*.cpp) $(wildcard *.cpp)
OBJ = ch_constructor.o nodes_and_edges.o unit_tests.o parser.o run_tests.o
DEPENDFILE = .depend

all: $(BIN)

ch_constructor: ch_constructor.o nodes_and_edges.o parser.o

run_tests: run_tests.o nodes_and_edges.o unit_tests.o parser.o

$(DEPENDFILE): $(SRCS)
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $^ > $(DEPENDFILE)

-include $(DEPENDFILE)

.PHONY: clean
clean:
	rm -rf $(BIN) $(OBJ) .depend
