# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

# Target executable name
TARGET = ./ME.exe

# Test object file
TEST_OBJ = test.obj

# Source files
SRC = ./ME.cpp

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET) 

# Run rule
run: all
	./$(TARGET) $(OBJ)

# Test rule (runs with test.obj)
test: all
	./$(TARGET) $(TEST_OBJ)

.PHONY: all clean run test
