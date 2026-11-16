CXX = g++
CXXFLAGS = -Wall -std=c++11 -Iinclude

SRCS = src/main.cpp src/GitObjects.cpp src/BareRemote.cpp src/Repository.cpp
TARGET = minigit.exe

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
