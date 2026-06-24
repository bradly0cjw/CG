# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

TARGETLAB = Lab13

# Target executable name
TARGET = ./$(TARGETLAB).exe

# Source files
SRC = ./shadow.cpp
SRC_ORIG = ./shadow_original.cpp

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Link Original
original: $(SRC_ORIG)
	$(CXX) $(SRC_ORIG) -o ./shadow_original.exe $(CXXFLAGS) $(LDFLAGS)
	./shadow_original.exe

# Clean rule
clean:
	rm -f $(TARGET)

# Run rule
run: all
	$(TARGET)

.PHONY: all clean run
