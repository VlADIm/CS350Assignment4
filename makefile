# Declaration of variables
CC = g++
CC_FLAGS = -std=c++17 -Wall -Wextra -pedantic -g 
L_FLAGS = -lpthread
 
# File names
EXEC = run
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
	 
	 # Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(L_FLAGS)

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@
# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
