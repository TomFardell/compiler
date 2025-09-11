CXX=g++
CPPFLAGS=-g3 -Wall
LDFLAGS=

FOLDER=src
EXE=compiler
OBJECTS=$(FOLDER)/lexer.o $(FOLDER)/compiler.o

default: $(EXE)

$(EXE) : $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(EXTRAFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CPPFLAGS) $(EXTRAFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXE)
