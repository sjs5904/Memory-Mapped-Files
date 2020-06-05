CC = gcc
CXX = g++
ECHO = echo
RM = rm -f

BIN = alloc prov-rep

all: $(BIN)

.DEFAULT_GOAL:=all

alloc: alloc.cpp
	$(CC) -o $@ $^

prov-rep: prov-rep.cpp
	$(CC) -o $@ $^

.PHONY: all clean clobber	

clean:
	@$(ECHO) Removing all generated files
	@$(RM) *.o $(BIN) *.d TAGS core vgcore.* gmon.out
	
clobber: clean
	@$(ECHO) Removing backup files
	@$(RM) *~ \#* *pgm