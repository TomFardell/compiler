CXX=g++
CPPFLAGS=-g3 -Wall -std=c++23
LDFLAGS=

FOLDER=src
EXE=compiler
OBJECTS=$(FOLDER)/lexer.o $(FOLDER)/parser.o $(FOLDER)/emitter.o $(FOLDER)/ast.o $(FOLDER)/compiler.o 
default: $(EXE)

$(EXE) : $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(EXTRAFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CPPFLAGS) $(EXTRAFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(EXE) *.out *.s *.asm
