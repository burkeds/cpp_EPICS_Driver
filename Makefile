# Compiler settings
CXX = /usr/bin/g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++20 -Iinclude -I/usr/local/epics/base/7-0-3/include -I/usr/local/epics/base/7-0-3/include/os/Linux -I/usr/local/epics/base/7-0-3/include/compiler/gcc/ -L/u

# Directories
SRC_DIR = src
INC_DIRS = include /usr/local/epics/base/7-0-3/include /usr/local/epics/base/7-0-3/include/os/Linux /usr/local/epics/base/7-0-3/include/compiler/gcc/
LIB_DIRS = /usr/local/epics/base/7-0-3/lib/linux-x86_64 lib

# Libraries
LIBS = Com ca

# Targets
TARGET = testEpicsProxy
SRCS = $(wildcard $(SRC_DIR)/*.cpp) testEpicsProxy.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/%.o,$(SRCS)) src/EpicsProxy.o

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(addprefix -I,$(INC_DIRS))

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean