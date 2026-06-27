# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include -Iinclude
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32 -lxinput9_1_0

TARGET = ./FinalProject.exe

# Source files
SRC = ./FE.cpp ./src/gltools.cpp ./src/math3d.cpp

# Default rule
all: $(TARGET)

# Link and compile
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)

# Run rule
run: all
	$(TARGET)

.PHONY: all clean run
