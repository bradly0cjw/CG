# Xbox Controller Support (set to 1 to enable, 0 to disable)
ENABLE_CONTROLLER ?= 0

# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -O2 -IC:\freeglut\include -Iinclude
LDFLAGS = -LC:\freeglut\lib\x64 -lfreeglut -lopengl32 -lglu32

ifeq ($(strip $(ENABLE_CONTROLLER)),1)
CXXFLAGS += -DENABLE_CONTROLLER=1
LDFLAGS += -lxinput9_1_0
else
CXXFLAGS += -DENABLE_CONTROLLER=0
endif

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
	-rm -f $(TARGET)

# Run rule
run: clean all 
	$(TARGET)

# Run with controller support
control: clean
	$(MAKE) ENABLE_CONTROLLER=1
	$(TARGET)

.PHONY: all clean run control
