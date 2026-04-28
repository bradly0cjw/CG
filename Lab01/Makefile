# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

# Source files
SRCS = demo.cpp nickolas_wild.cpp rec.cpp temp1.cpp
TARGETS = $(SRCS:.cpp=.exe)

# Default rule
all: $(TARGETS)

# Link
%.exe: %.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)

# Clean rule
clean:
	rm -f $(TARGETS)

.PHONY: all clean
