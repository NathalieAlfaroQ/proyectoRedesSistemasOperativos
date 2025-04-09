# Makefile

CXX = g++
CXXFLAGS = -Wall -O2
TARGET = lector_figura
SRC = lector_figura.cc

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
