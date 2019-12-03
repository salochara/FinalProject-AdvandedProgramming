# Makefile to compile a complex program composed of many libraries

### Variables for this project ###
# These should be the only ones that need to be modified
# The files that must be compiled, with a .o extension
OBJECTS = FirstTest.o
# The header files
#DEPENDS = vigenere.h
# The executable program to be created
MAIN = first_test

### Variables for the compilation rules ###
# These should work for most projects, but can be modified when necessary
# The compiler program to use
CC = gcc
# Options to use when compiling object files
CFLAGS = -Wall -g -std=c99 -pedantic # -O2
# Options to use for the final linking process
# This one links the math library
LDLIBS = -lm

### The rules ###
# These should work for most projects without change
# Special variable meanings:
#   $@  = The name of the rule
#   $^  = All the requirements for the rule
#   $<  = The first required file of the rule

# Default rule
all: $(MAIN)

# Rule to make the executable
$(MAIN): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Rule to make the object files
%.o: %.c $(DEPENDS)
	$(CC) $< -c -o $@ $(CFLAGS)

# Clear the compiled files
clean:
	rm -rf *.o $(MAIN)

# Create a zip with the source code of the project
# Useful for submitting assignments
# Will clean the compilation first
zip: clean
	zip -r $(MAIN).zip *

# Indicate the rules that do not refer to a file
.PHONY: clean all zip