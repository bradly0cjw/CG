# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

# Target executable name
TARGET = ./lab11.exe

# Source files
SRC = ./lab11.cpp
TEST_SRC = ./test.cpp
TEST_TARGET = ./test.exe

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)

# Run rule
run: all
	$(TARGET)

# Test rule
test: $(TEST_TARGET)
$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(TEST_SRC) -o $(TEST_TARGET) $(CXXFLAGS) $(LDFLAGS)
	$(TEST_TARGET)

.PHONY: all clean run test
