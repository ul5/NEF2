CXX := gcc
INCLUDE_DIRS:=-I.
LIBRARY_DIRS:=-L.
LIBRARY_FILES:=
OBJS:=NEF.o

all: clean NEF

clean:
	-rm -f *.o
	-rm -f NEF

NEF: $(OBJS)
	$(CXX)  -g -o NEF $(OBJS)

%.o: $.c
	$(CXX) -Wp -w -c -g $(INCLUDE_DIRS) $(LIBRARY_DIRS) $(LIBRARY_FILES) -o @.o $.c
