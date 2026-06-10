# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

TARGETLAB = Lab12

# Target executable name
TARGET = ./$(TARGETLAB).exe

# Source files
SRC = ./$(TARGETLAB).cpp

DEMOSRC = ./demo/openglLightingExample.cpp
DEMOTARGET = ./demo/openglLightingExample.exe

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGET)
	rm -f $(DEMOTARGET)

# Run rule
run: all
	$(TARGET)

demo: $(DEMOTARGET)
$(DEMOTARGET): $(DEMOSRC)
	$(CXX) $(DEMOSRC) -o $(DEMOTARGET) $(CXXFLAGS) $(LDFLAGS)
	$(DEMOTARGET)

.PHONY: all clean run demo