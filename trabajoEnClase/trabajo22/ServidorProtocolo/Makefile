# Makefile limpio y optimizado

CXX = g++
CXXFLAGS = -Wall -g

# Ejecutables
TARGETS = ServidorTenedor.out ServidorFiguras.out HTTPCliente.out

all: $(TARGETS)

# Reglas para compilar los ejecutables

ServidorTenedor.out: ServidorTenedor.cc Socket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

ServidorFiguras.out: ServidorFiguras.cc Socket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

HTTPCliente.out: HTTPCliente.cc Socket.cc VSocket.cc FileSystem.cc
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGETS) FileSystem.bin
