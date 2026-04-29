# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

# Target executable name
TARGET = ./lab6.exe

# Source files
SRC = ./lab6.cpp

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
	$(TARGET)

.PHONY: all clean run
