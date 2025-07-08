#include <iostream>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <sys/types.h>
#include <ifaddrs.h>
#include <vector>
#include <dirent.h>
#include <sstream>
#include "Socket.h"
#include "FileSystem.h"

#define NETWORK_INTERFACE "eth0"
#define UDP_PORT_SEND 4321
#define UDP_PORT_RECEIVE 1234
#define TCP_PORT 5018
#define BUFSIZE 1024

// Variables globales
bool running = true;
std::vector<std::string> broadcastAddresses;  // Vector para almacenar todas las direcciones de broadcast
FileSystem* fileSystem = nullptr;  // Sistema de archivos para almacenar figuras

// Función para inicializar el vector de direcciones de broadcast
void initBroadcastAddresses() {
    broadcastAddresses.push_back("172.16.123.15");  // Isla 0
    broadcastAddresses.push_back("172.16.123.31");  // Isla 1
    broadcastAddresses.push_back("172.16.123.47");  // Isla 2
    broadcastAddresses.push_back("172.16.123.63");  // Isla 3
    broadcastAddresses.push_back("172.16.123.79");  // Isla 4
    broadcastAddresses.push_back("172.16.123.95");  // Isla 5
    broadcastAddresses.push_back("172.16.123.111"); // Isla 6
    broadcastAddresses.push_back("172.28.47.255");  // Isla 7 (casa)

}

// Función para obtener la IP local priorizando direcciones 172.x.x.x
std::string obtenerIP() {
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
    std::string ip_final = "127.0.0.1"; // Valor por defecto
    std::string ip_alternativa = "";    // Para almacenar cualquier IP no-loopback encontrada

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET &&
            std::string(ifa->ifa_name) != "lo") {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            std::string current_ip = std::string(addr);
            
            // Guardar cualquier IP no-loopback como respaldo
            if (ip_alternativa.empty()) {
                ip_alternativa = current_ip;
            }
            
            // Si encontramos una IP que comienza con "172.", la usamos inmediatamente
            if (current_ip.substr(0, 4) == "172.") {
                ip_final = current_ip;
                break;  // Terminar búsqueda, encontramos la IP deseada
            }
        }
    }
    
    // Si no encontramos una IP que empiece con "172." pero sí encontramos otra IP no-loopback
    if (ip_final == "127.0.0.1" && !ip_alternativa.empty()) {
        ip_final = ip_alternativa;
    }
    
    freeifaddrs(ifap);
    return ip_final;
}

// Función para manejar la señal SIGINT (Ctrl+C)
void manejar_senal(int signo) {
    // Crear socket UDP para enviar mensajes de apagado
    Socket udpSocket('d', false);
    udpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    udpSocket.Bind(UDP_PORT_SEND);
    udpSocket.EnableUDP();

    if (signo == SIGINT) {
        // Indicar que el servidor se está apagando
        running = false;
        std::string ip_local = obtenerIP();
        std::cout << std::endl;
        std::string response = "BEGIN/OFF/SERVIDOR/" + ip_local + "/" + std::to_string(TCP_PORT) + "/END";
        std::cout << response << std::endl;

        // Enviar mensaje de apagado a todas las islas
        for (const auto& broadcastAddr : broadcastAddresses) {
            udpSocket.SendUDP(response.c_str(), response.size(), broadcastAddr.c_str(), UDP_PORT_SEND);
            std::cout << "[DEBUG] Mensaje de apagado enviado a " << broadcastAddr << std::endl;
        }

        // Terminar el programa
        exit(0);  
    }
}

// Hilo para enviar mensajes UDP periódicos (anunciarse)
void udpBroadcaster() {
    // Crear socket UDP para enviar mensajes de broadcast
    Socket udpSocket('d', false);
    udpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    udpSocket.Bind(UDP_PORT_SEND);
    udpSocket.EnableUDP();

    std::string ip_local = obtenerIP();
    std::string mensaje = "BEGIN/ON/SERVIDOR/" + ip_local + "/" + std::to_string(TCP_PORT) + "/END";
    
    std::cout << "[DEBUG] Enviando broadcasts a todas las islas" << std::endl;
    
    // Recorrer todas las direcciones de broadcast
    for (const auto& broadcastAddr : broadcastAddresses) {
        // Enviar el mensaje de broadcast
        udpSocket.SendUDP(mensaje.c_str(), mensaje.size(), broadcastAddr.c_str(), UDP_PORT_SEND);
        std::cout << "[DEBUG] Broadcast enviado a " << broadcastAddr << ": " << mensaje << std::endl;
        // Pequeña pausa entre envíos para no saturar la red
        usleep(100000); // 100ms
    }
    
}

// Hilo para recibir mensajes UDP (descubrimiento)
void udpListener() {
    // Crear socket UDP para recibir mensajes
    Socket udpSocket('d', false);
    udpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    udpSocket.Bind(UDP_PORT_RECEIVE);

    char buffer[BUFSIZE];
    char senderIP[INET6_ADDRSTRLEN]; // Mayor tamaño para compatibilidad con IPv6
    int senderPort;
    
    while (running) {
        memset(buffer, 0, BUFSIZE);
        size_t bytesReceived = udpSocket.ReceiveUDP(buffer, BUFSIZE, senderIP, &senderPort);
        buffer[bytesReceived] = '\0';
        
        std::string mensaje(buffer);
        std::cout << "[DEBUG] Mensaje recibido de " << senderIP << ":" << senderPort << ": " << mensaje << std::endl;

        // Si es un mensaje del tenedor
        if (mensaje.find("BEGIN/ON/TENEDOR/") != std::string::npos) {
            // Extraer IP y puerto del tenedor
            size_t ipPos = mensaje.find("TENEDOR/") + 8;
            size_t endIpPos = mensaje.find("/", ipPos);
            std::string tenedorIP = mensaje.substr(ipPos, endIpPos - ipPos);
            
            size_t portPos = endIpPos + 1;
            size_t endPortPos = mensaje.find("/END", portPos);
            int tenedorPort = std::stoi(mensaje.substr(portPos, endPortPos - portPos));

            std::cout << "[DEBUG] Tenedor detectado en " << tenedorIP << ":" << tenedorPort << std::endl;
            
            // Anunciarse de nuevo utilizando la función de broadcast existente
            std::thread broadcastThread(udpBroadcaster);
            broadcastThread.join();
        }
        // Si es un mensaje de apagado del tenedor
        else if (mensaje.find("BEGIN/OFF/TENEDOR/") != std::string::npos) {
            // Extraer IP del tenedor
            size_t ipPos = mensaje.find("TENEDOR/") + 8;
            size_t endIpPos = mensaje.find("/", ipPos);
            std::string tenedorIP = mensaje.substr(ipPos, endIpPos - ipPos);

            std::cout << "[DEBUG] El tenedor " << tenedorIP << " se ha desconectado" << std::endl;
        }
    }
}

// Función para cargar las figuras iniciales
void cargarFigurasIniciales() {
    // Crear directorio para figuras si no existe
    system("mkdir -p Figuras");
    
    // Buscar archivos .txt en el directorio de figuras
    DIR* dir = opendir("Figuras");
    if (dir) {
        struct dirent* entry;
        int countFiguras = 0;
        while ((entry = readdir(dir)) != nullptr) {
            std::string nombre = entry->d_name;
            if (nombre.size() > 4 && nombre.substr(nombre.size() - 4) == ".txt") {
                // Mantener el nombre con la extensión .txt
                fileSystem->Agregar_Figura(nombre);
                countFiguras++;
            }
        }
        closedir(dir);
        
        if (countFiguras > 0) {
            std::cout << "[DEBUG] Figuras cargadas: " << countFiguras << std::endl;
        }
    }
}

// Función para manejar una conexión cliente y responder según el protocolo
void manejarRespuestaFileSystem(Socket* clientSocket) {
    char buffer[BUFSIZE];
    memset(buffer, 0, BUFSIZE);
    
    // Leer la solicitud del cliente
    size_t bytesRead = clientSocket->Read(buffer, BUFSIZE);
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    std::cout << "[DEBUG] Solicitud recibida: " << request << std::endl;
    
    std::string response;
    
    // Procesar solicitud de lista de objetos
    if (request == "BEGIN/OBJECTS/END") {
        std::cout << "[DEBUG] Procesando solicitud de lista de objetos..." << std::endl;

        // Obtener la lista de dibujos usando Listar_Figuras
        std::string listaFiguras = fileSystem->Listar_Figuras();
        
        // Verificar si hay figuras disponibles
        if (listaFiguras == "[ERROR] No hay figuras disponibles\n") {
            // No hay dibujos disponibles
            response = "BEGIN/ERROR/104/El servidor de dibujos se encuentra vacío/END";
        } else {
            // Construir la lista de dibujos separada por saltos de línea
            response = "BEGIN/OK/";
            
            // Eliminar el último salto de línea si existe
            if (!listaFiguras.empty() && listaFiguras[listaFiguras.size() - 1] == '\n') {
                listaFiguras.pop_back();
            }
            
            // Procesar cada línea y eliminar la extensión .txt
            std::string nombreFigura;
            std::string listaFinal;
            std::istringstream stream(listaFiguras);

            bool primeraLinea = true;
            while (std::getline(stream, nombreFigura)) {
                // Eliminar la extensión .txt si existe
                if (nombreFigura.size() > 4 && nombreFigura.substr(nombreFigura.size() - 4) == ".txt") {
                    nombreFigura = nombreFigura.substr(0, nombreFigura.size() - 4);
                }
                
                if (!primeraLinea) {
                    listaFinal += "\n";
                }
                listaFinal += nombreFigura;
                primeraLinea = false;
            }
            
            response += listaFinal + "/END";
        }
    }
    // Procesar solicitud de recurso
    else if (request.find("BEGIN/GET/") == 0 && request.find("/END") != std::string::npos) {
        // Extraer el nombre del dibujo
        std::string nombreDibujo = request.substr(10, request.find("/END") - 10);
        std::cout << "[DEBUG] Dibujo solicitado: " << nombreDibujo << std::endl;

        // Añadir la extensión .txt si no existe
        if (nombreDibujo.size() <= 4 || nombreDibujo.substr(nombreDibujo.size() - 4) != ".txt") {
            nombreDibujo += ".txt";
        }
        // Leer la figura del sistema de archivos
        // La implementación de Leer_Figura maneja internamente la extensión .txt
        std::string contenidoDibujo = fileSystem->Leer_Figura(nombreDibujo);
        
        // Verificar si la figura existe
        if (contenidoDibujo.find("[INFO] Figura '") == 0) {
            // El dibujo no existe
            response = "BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END";
        } else {
            // Eliminar caracteres nulos al final del contenido
            size_t posNulo = contenidoDibujo.find('\0');
            if (posNulo != std::string::npos) {
                contenidoDibujo = contenidoDibujo.substr(0, posNulo);
            }
            
            // Construir la respuesta
            response = "BEGIN/OK/" + contenidoDibujo + "/END";
        }
    }
    // Solicitud desconocida
    else {
        response = "BEGIN/ERROR/400/Solicitud no reconocida/END";
    }
    
    // Enviar la respuesta al cliente
    std::cout << "[DEBUG] Enviando respuesta: " << response << std::endl;
    clientSocket->Write(response.c_str(), response.size());

    // Cerrar la conexión con el cliente
    delete clientSocket;
}

// Hilo para atender conexiones TCP del tenedor
void tenedorHandler() {
    // Crear socket TCP para atender conexiones
    Socket tcpSocket('s', false);
    tcpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    tcpSocket.Bind(TCP_PORT);
    tcpSocket.MarkPassive(5);  // Queue de 5 conexiones pendientes

    std::cout << "[DEBUG] Servidor TCP iniciado en puerto " << TCP_PORT << std::endl;
    
    while (running) {
        // Esperar por una nueva conexión
        Socket* clientSocket = (Socket*)tcpSocket.AcceptConnection();
        if (clientSocket) {
            // Crear un nuevo hilo para atender a este cliente
            std::thread clientThread(manejarRespuestaFileSystem, clientSocket);
            clientThread.detach();  // No esperamos a que termine
        }
    }
}

// Función principal
int main(int argc, char* argv[]) {
    // Registrar manejador de señal para SIGINT (Ctrl+C)
    signal(SIGINT, manejar_senal);
    
    // Inicializar el vector de direcciones de broadcast
    initBroadcastAddresses();

    std::cout << "[INFO] Total de direcciones de broadcast: " << broadcastAddresses.size() << std::endl;

    // Inicializar el sistema de archivos
    fileSystem = new FileSystem("FileSystem.bin");
    fileSystem->Crear_FileSystem();

    // Cargar figuras iniciales
    cargarFigurasIniciales();

    // Iniciar hilos para UDP
    std::thread udpListenerThread(udpListener);
    std::thread udpBroadcasterThread(udpBroadcaster);
    std::thread tcpServerThread(tenedorHandler);

    // Esperar a que terminen los hilos (nunca deberían terminar)
    udpListenerThread.join();
    udpBroadcasterThread.join();
    tcpServerThread.join();

    return 0;
}