CXX=g++
CPPFLAGS=-g3 -Wall -std=c++23
LDFLAGS=

ASM=nasm
ASMFLAGS=-g -f elf64
CC=gcc
CCFLAGS=-g -no-pie

FOLDER=src
EXE=compiler
OBJECTS=$(FOLDER)/lexer.o $(FOLDER)/parser.o $(FOLDER)/emitter.o $(FOLDER)/ast.o $(FOLDER)/compiler.o 

.PHONY: default clean asm-clean

default: $(EXE)

clean:
	rm -f $(OBJECTS) $(EXE) *.out *.s *.asm *.o

asm-clean:
	rm -f *.asm *.o *.out 

%.out: %.asm
	$(ASM) $(ASMFLAGS) -o $*.o $<
	$(CC) $(CCFLAGS) -o $@ $*.o 
	rm -f $*.o

$(EXE): $(OBJECTS)
	$(CXX) $(CPPFLAGS) $(EXTRAFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CXX) $(CPPFLAGS) $(EXTRAFLAGS) -c -o $@ $<
