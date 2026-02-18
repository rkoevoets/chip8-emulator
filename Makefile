CXX=g++
CXXFLAGS=-Iinclude -std=c++20 -Wall

OUTFILE=chip8


SRCS=$(shell find -L src/ -type f -name "*.cpp" -print)
HEADERS=$(shell find -L src/ -type f -name "*.h" -print)
OBJS=$(SRCS:%.cpp=%.o)

all: $(OUTFILE)

$(OUTFILE): $(OBJS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(OUTFILE) -lSDL2

%.o: %.c
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm $(OBJS) $(OUTFILE)

test:
	@echo $(SRCS)
	@echo $(OBJS)