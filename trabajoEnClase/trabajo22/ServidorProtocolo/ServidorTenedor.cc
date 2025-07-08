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
#include <map>
#include <sstream>
#include <iomanip>
#include "Socket.h"

#define NETWORK_INTERFACE "eth0"
#define UDP_PORT_SEND 1234
#define UDP_PORT_RECEIVE 4321
#define TCP_PORT_CLIENTE 5015
#define BUFSIZE 1024

// Estructura para almacenar información sobre un servidor de figuras
struct ServidorInfo {
    std::string ip;
    int puerto;
    std::vector<std::string> figuras;  // Lista de figuras que ofrece este servidor
    time_t ultimoAnuncio;  // Timestamp del último anuncio recibido
};

// Estructura para almacenar información sobre una figura
struct FiguraInfo {
    std::string nombre;
    std::string servidorIP;
    int servidorPuerto;
};

// Variables globales
bool running = true;
std::vector<std::string> broadcastAddresses;  // Vector para almacenar todas las direcciones de broadcast
std::map<std::string, ServidorInfo> servidoresFiguras;  // Mapa de servidores de figuras (IP -> info)
std::map<std::string, FiguraInfo> tablaDeFiguras;  // Tabla de enrutamiento (nombre figura -> info)

/**
 * Limpia las etiquetas del protocolo de un mensaje recibido
 * 
 * @param mensaje El mensaje completo con etiquetas del protocolo
 * @return El contenido del mensaje sin etiquetas
 */
std::string limpiarEtiquetasProtocolo(const std::string& mensaje) {
    // Buscar patrones comunes de inicio y fin
    size_t inicioContenido = 0;
    size_t finContenido = mensaje.length();
    
    // Buscar patrones de inicio
    const std::vector<std::string> patronesInicio = {
        "BEGIN/OK/", "BEGIN/ON/SERVIDOR/", 
        "BEGIN/ON/TENEDOR/", "BEGIN/OFF/SERVIDOR/", "BEGIN/OFF/TENEDOR/",
        "BEGIN/GET/", "BEGIN/OBJECTS/"
    };
    
    for (const auto& patron : patronesInicio) {
        size_t pos = mensaje.find(patron);
        if (pos != std::string::npos) {
            inicioContenido = pos + patron.length();
            break;
        }
    }
    
    // Buscar patrón de fin
    size_t pos = mensaje.find("/END");
    if (pos != std::string::npos) {
        finContenido = pos;
    }
    
    // Extraer solo el contenido
    if (inicioContenido < finContenido) {
        return mensaje.substr(inicioContenido, finContenido - inicioContenido);
    }
    
    // Si no se encontró patrón, devolver el mensaje original
    return mensaje;
}

/**
 * Actualiza la tabla de ruteo solicitando figuras a un servidor específico
 * 
 * @param serverIP Dirección IP del servidor de figuras
 * @param serverPort Puerto TCP del servidor de figuras
 * @return true si la operación fue exitosa, false en caso contrario
 */
bool actualizarTabla(const std::string& serverIP, int serverPort) {
    const int MAX_INTENTOS = 3;
    int intentos = 0;
    bool conectado = false;
    Socket tcpSocket('s', false);
    tcpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    
    // Intentar conexión con reintentos
    while (!conectado && intentos < MAX_INTENTOS) {
        intentos++;
        std::cout << "[INFO] Intento " << intentos << " de conectar a " << serverIP
            << ":" << serverPort << " para solicitar figuras..." << std::endl;
        
        if (tcpSocket.MakeConnection(serverIP.c_str(), serverPort) >= 0) {
            conectado = true;
            std::cout << "[INFO] Conexión establecida con " << serverIP << ":" << serverPort << std::endl;
        } else {
            std::cerr << "[ERROR] No se pudo conectar en el intento " << intentos << std::endl;
            sleep(2); // Esperar 2 segundos antes de reintentar
        }
    }
    
    if (!conectado) {
        std::cerr << "[ERROR] No se pudo conectar a " << serverIP << ":" << serverPort
            << " después de " << MAX_INTENTOS << " intentos." << std::endl;
        return false;
    }
    
    // Enviar mensaje de solicitud según protocolo
    std::string request = "BEGIN/OBJECTS/END";
    tcpSocket.Write(request.c_str(), request.length());
    std::cout << "[DEBUG] Solicitud enviada: " << request << std::endl;
    
    // Leer respuesta
    char buffer[BUFSIZE];
    memset(buffer, 0, BUFSIZE);
    size_t bytesRead = tcpSocket.Read(buffer, BUFSIZE - 1);
    buffer[bytesRead] = '\0';
    
    // Procesar la respuesta
    std::string response(buffer);
    //std::cout << "[DEBUG] Respuesta recibida: " << response << std::endl;
    
    // Verificar si es una respuesta válida
    if (response.find("BEGIN/OK") == 0) {
        // Limpiar la respuesta de etiquetas del protocolo
        std::string figurasList = limpiarEtiquetasProtocolo(response);
        
        // Limpiar lista previa de figuras para este servidor
        auto& servidor = servidoresFiguras[serverIP];
        servidor.ip = serverIP;
        servidor.puerto = serverPort;
        servidor.ultimoAnuncio = time(nullptr);
        servidor.figuras.clear();
        
        // Si hay figuras, procesarlas
        if (!figurasList.empty()) {
            std::istringstream stream(figurasList);
            std::string figura;
            
            std::cout << "[INFO] Figuras disponibles en " << serverIP << ":" << serverPort << ":" << std::endl;
            
            // Procesar cada figura línea por línea
            while (std::getline(stream, figura)) {
                if (!figura.empty()) {
                    // Eliminar caracteres de retorno si están presentes
                    if (figura.back() == '\r') {
                        figura.pop_back();
                    }
                    
                    std::cout << "  - " << figura << std::endl;
                    
                    // Actualizar la lista de figuras del servidor
                    servidor.figuras.push_back(figura);
                    
                    // Actualizar la tabla de enrutamiento
                    tablaDeFiguras[figura] = {figura, serverIP, serverPort};
                }
            }
            
            // Caso especial: una sola figura sin saltos de línea
            if (servidor.figuras.empty() && !figurasList.empty()) {
                std::string figura = figurasList;
                if (figura.back() == '\r') {
                    figura.pop_back();
                }
                
                std::cout << "  - " << figura << std::endl;
                
                // Actualizar la lista de figuras del servidor
                servidor.figuras.push_back(figura);
                
                // Actualizar la tabla de enrutamiento
                tablaDeFiguras[figura] = {figura, serverIP, serverPort};
            }
        } else {
            std::cout << "[INFO] El servidor no tiene figuras disponibles" << std::endl;
        }
        tcpSocket.Close();
        return true;
    } 
    else if (response.find("BEGIN/ERROR/104") != std::string::npos) {
        // El servidor no tiene figuras
        std::cout << "[INFO] El servidor " << serverIP << " no tiene figuras disponibles" << std::endl;
        
        // Actualizar la información del servidor pero con lista vacía
        auto& servidor = servidoresFiguras[serverIP];
        servidor.ip = serverIP;
        servidor.puerto = serverPort;
        servidor.ultimoAnuncio = time(nullptr);
        servidor.figuras.clear();
        
        tcpSocket.Close();
        return true;
    }
    else {
        std::cerr << "[ERROR] Respuesta desconocida o inválida del servidor" << std::endl;
        tcpSocket.Close();
        return false;
    }
}

/**
 * Solicita una figura específica al servidor que la contiene
 * 
 * @param nombreFigura Nombre de la figura a solicitar
 * @return Contenido de la figura o mensaje de error
 */
std::string solicitarFigura(const std::string& nombreFigura) {
    // 1. Verificar si la figura existe en la tabla de rutas
    if (tablaDeFiguras.find(nombreFigura) == tablaDeFiguras.end()) {
        std::cerr << "[ERROR] La figura '" << nombreFigura << "' no está en la tabla de rutas." << std::endl;
        return "[ERROR] La figura '" + nombreFigura + "' no se encuentra registrada.";
    }
    
    // 2. Obtener información del servidor desde la tabla de rutas
    const auto& infoFigura = tablaDeFiguras[nombreFigura];
    std::string serverIP = infoFigura.servidorIP;
    int serverPort = infoFigura.servidorPuerto;
    
    std::cout << "[INFO] Figura '" << nombreFigura << "' encontrada en servidor " 
              << serverIP << ":" << serverPort << std::endl;
    
    // 3. Establecer conexión con el servidor
    const int MAX_INTENTOS = 3;
    int intentos = 0;
    bool conectado = false;
    Socket tcpSocket('s', false);
    tcpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    
    while (!conectado && intentos < MAX_INTENTOS) {
        intentos++;
        std::cout << "[INFO] Intento " << intentos << " de conectar a " << serverIP
            << ":" << serverPort << " para solicitar figura..." << std::endl;
        
        if (tcpSocket.MakeConnection(serverIP.c_str(), serverPort) >= 0) {
            conectado = true;
            std::cout << "[INFO] Conexión establecida con " << serverIP << ":" << serverPort << std::endl;
        } else {
            std::cerr << "[ERROR] No se pudo conectar en el intento " << intentos << std::endl;
            sleep(2); // Esperar 2 segundos antes de reintentar
        }
    }
    
    if (!conectado) {
        return "[ERROR] No se pudo conectar al servidor " + serverIP + ":" + std::to_string(serverPort);
    }
    
    // 4. Enviar solicitud de figura específica según el protocolo
    std::string request = "BEGIN/GET/" + nombreFigura + "/END";
    tcpSocket.Write(request.c_str(), request.length());
    std::cout << "[DEBUG] Solicitud enviada: " << request << std::endl;
    
    // 5. Leer respuesta (múltiples bloques si es necesario)
    std::string respuestaCompleta;
    bool finRespuesta = false;
    int bloquesLeidos = 0;
    size_t totalBytesLeidos = 0;
    char buffer[BUFSIZE];
    
    while (!finRespuesta) {
        memset(buffer, 0, BUFSIZE);
        size_t bytesRead = tcpSocket.Read(buffer, BUFSIZE - 1);
        if (bytesRead <= 0) {
            // No hay más datos o error de lectura
            break;
        }
        buffer[bytesRead] = '\0';
        totalBytesLeidos += bytesRead;
        bloquesLeidos++;
        // Añadir el bloque a la respuesta completa
        respuestaCompleta += buffer;
        // Verificar si hemos encontrado el final de la respuesta
        if (respuestaCompleta.find("/END") != std::string::npos) {
            finRespuesta = true;
        }
    }
    tcpSocket.Close();
    // 6. Procesar respuesta
    if (respuestaCompleta.find("BEGIN/OK/") == 0) {
        // Extraer el contenido entre GET/ y /END
        size_t posInicio = respuestaCompleta.find("GET/");
        if (posInicio != std::string::npos) {
            posInicio += 4; // Avanzar después de "GET/"
            size_t posFin = respuestaCompleta.find("/END", posInicio);
            if (posFin != std::string::npos) {
                std::string contenido = respuestaCompleta.substr(posInicio, posFin - posInicio);
                return contenido;
            }
        }
        // Si no se encuentra el patrón específico, usar el limpiador genérico
        std::string contenido = limpiarEtiquetasProtocolo(respuestaCompleta);
        return contenido;
    } 
    else if (respuestaCompleta.find("BEGIN/ERROR/") != std::string::npos) {
        std::string errorMsg = limpiarEtiquetasProtocolo(respuestaCompleta);
        std::cerr << "[ERROR] El servidor reportó un error: " << errorMsg << std::endl;
        return "[ERROR] " + errorMsg;
    }
    else {
        std::cerr << "[ERROR] Respuesta desconocida del servidor" << std::endl;
        return "[ERROR] Respuesta desconocida del servidor";
    }
}

/**
 * Función auxiliar para manejar solicitudes HTTP de un cliente
 * @param clientSocket Socket conectado al cliente
 */
void manejarSolicitudHTTP(Socket* clientSocket) {
    // 1. Leer la solicitud
    char buffer[BUFSIZE];
    memset(buffer, 0, BUFSIZE);
    size_t bytesRead = clientSocket->Read(buffer, BUFSIZE - 1);
    
    if (bytesRead <= 0) {
        std::cerr << "[ERROR] Error al leer la solicitud del cliente" << std::endl;
        delete clientSocket;
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    std::cout << "[DEBUG] Solicitud HTTP recibida: " << request << std::endl;
    
    // 2. Analizar la solicitud HTTP
    std::string path;
    
    // Extraer la ruta de la primera línea
    size_t startPos = request.find("GET ");
    size_t endPos = request.find(" HTTP/1.1");
    
    if (startPos != std::string::npos && endPos != std::string::npos) {
        path = request.substr(startPos + 4, endPos - (startPos + 4));
        std::cout << "[DEBUG] Path solicitado: " << path << std::endl;
    } else {
        std::cerr << "[ERROR] Formato de solicitud HTTP inválido" << std::endl;
        clientSocket->Write("HTTP/1.1 400 Bad Request\r\n\r\nFormato de solicitud inválido\n", 58);
        delete clientSocket;
        return;
    }
    
    // 3. Procesar según la ruta solicitada
    
    // Manejo de solicitud de listado de figuras
    if (path == "/listado") {
        std::cout << "[INFO] Procesando solicitud de listado de figuras" << std::endl;
        
        if (tablaDeFiguras.empty()) {
            std::string cuerpo = "<html><head><meta charset=\"utf-8\"></head><body><pre>\nNo hay figuras disponibles\n</pre></body></html>\n";
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << cuerpo.size() << "\r\n"
                << "\r\n"
                << cuerpo;
            clientSocket->Write(ss.str().c_str(), ss.str().length());
        } else {
            // Formato para mostrar la lista en forma legible
            std::string lista = "FIGURAS DISPONIBLES:\n\n";
            int contador = 1;
            
            for (const auto& [nombreFigura, info] : tablaDeFiguras) {
                lista += std::to_string(contador) + ". " + nombreFigura + 
                            " (servidor: " + info.servidorIP + ":" + 
                            std::to_string(info.servidorPuerto) + ")\n";
                contador++;
            }
            
            // Genera respuesta HTML con el listado dentro de <pre>
            std::string cuerpo = "<html><head><meta charset=\"utf-8\"></head><body><pre>\n" + 
                                    lista + 
                                    "\n</pre></body></html>\n";
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << cuerpo.size() << "\r\n"
                << "\r\n"
                << cuerpo;
            clientSocket->Write(ss.str().c_str(), ss.str().length());
        }
    }
    
    // Manejo de petición para obtener una figura específica
    else if (path.find("/figura=") == 0) {
        std::string nombreFigura = path.substr(8); // Omitir "/figura="
        std::cout << "[INFO] Cliente solicita figura: " << nombreFigura << std::endl;
        
        // Solicitar la figura usando la función existente
        std::string contenidoFigura = solicitarFigura(nombreFigura);
        
        if (contenidoFigura.find("[ERROR]") == 0) {
            // Si no se encontró la figura, responder 404
            std::string response = "HTTP/1.1 404 Not Found\r\n\r\nFigura no encontrada\n";
            clientSocket->Write(response.c_str(), response.length());
        } else {
            // Genera respuesta HTML con la figura dentro de <pre>
            std::string cuerpo = "<html><head><meta charset=\"utf-8\"></head><body><pre>\n" + 
                                    contenidoFigura + 
                                    "\n</pre></body></html>\n";
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << cuerpo.size() << "\r\n"
                << "\r\n"
                << cuerpo;
            clientSocket->Write(ss.str().c_str(), ss.str().length());
        }
    }
    
    // Ruta no reconocida
    else {
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\n"
                                "<html><head><meta charset=\"utf-8\"></head>"
                                "<body><h1>404 Not Found</h1><p>La página solicitada no existe.</p></body></html>\n";
        clientSocket->Write(response.c_str(), response.length());
    }
    
    // 4. Cerrar la conexión
    std::cout << "[INFO] Respuesta HTTP enviada al cliente" << std::endl;
    delete clientSocket;
    
}

/**
 * Hilo para atender conexiones TCP de clientes
 */
// Hilo para atender conexiones TCP del tenedor
void clienteHandler() {
    // Crear socket TCP para atender conexiones
    Socket tcpSocket('s', false);
    tcpSocket.Bind(TCP_PORT_CLIENTE);
    tcpSocket.MarkPassive(5);  // Queue de 5 conexiones pendientes

    std::cout << "[DEBUG] Servidor TCP iniciado en puerto " << TCP_PORT_CLIENTE << std::endl;

    while (running) {
        // Esperar por una nueva conexión
        Socket* clientSocket = (Socket*)tcpSocket.AcceptConnection();
        if (clientSocket) {
            // Crear un nuevo hilo para atender a este cliente
            std::thread clientThread(manejarSolicitudHTTP, clientSocket);
            clientThread.detach();  // No esperamos a que termine
        }
    }
}

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

// Manejador de señal para finalizar limpiamente
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
        std::string response = "BEGIN/OFF/TENEDOR/" + ip_local + "/" + std::to_string(UDP_PORT_SEND) + "/END";
        std::cout << response << std::endl;

        // Enviar mensaje de apagado a todas las islas
        for (const auto& broadcastAddr : broadcastAddresses) {
            udpSocket.SendUDP(response.c_str(), response.size(), broadcastAddr.c_str(), UDP_PORT_SEND);
            std::cout << "[DEBUG] Mensaje de apagado enviado a " << broadcastAddr << std::endl;
        }
        exit(0);
    }
}

// Hilo para recibir anuncios UDP de servidores de figuras
void udpListener() {
    // Crear socket UDP para recibir mensajes
    Socket udpSocket('d', false);
    udpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    udpSocket.Bind(UDP_PORT_RECEIVE);

    char buffer[BUFSIZE];
    char senderIP[INET6_ADDRSTRLEN]; // Usar el tamaño mayor para compatibilidad con IPv6
    int senderPort;
    
    while (running) {
        memset(buffer, 0, BUFSIZE);
        size_t bytesReceived = udpSocket.ReceiveUDP(buffer, BUFSIZE, senderIP, &senderPort);
        buffer[bytesReceived] = '\0';
        
        std::string mensaje(buffer);
        std::cout << "[DEBUG] Recibido mensaje de " << senderIP << ":" << senderPort << ": " << mensaje << std::endl;

        // Procesar el mensaje
        if (mensaje.find("BEGIN/ON/SERVIDOR/") != std::string::npos) {
            // Extraer IP y puerto del servidor
            size_t ipPos = mensaje.find("SERVIDOR/") + 9;
            size_t endIpPos = mensaje.find("/", ipPos);
            std::string serverIP = mensaje.substr(ipPos, endIpPos - ipPos);
            
            size_t portPos = endIpPos + 1;
            size_t endPortPos = mensaje.find("/END", portPos);
            int serverPort = std::stoi(mensaje.substr(portPos, endPortPos - portPos));
            
            // Mostrar información del servidor detectado
            std::cout << "[DEBUG] Servidor detectado en " << serverIP << ":" << serverPort << std::endl;

            // Solicitar la lista de figuras en un hilo separado
            std::thread([serverIP, serverPort]() {
                // Usamos el puerto del servidor para la comunicación TCP
                actualizarTabla(serverIP, serverPort);
            }).detach();
        }
        else if (mensaje.find("BEGIN/OFF/SERVIDOR/") != std::string::npos) {
            // Extraer IP y puerto del servidor que se apaga
            size_t ipPos = mensaje.find("SERVIDOR/") + 9;
            size_t endIpPos = mensaje.find("/", ipPos);
            std::string serverIP = mensaje.substr(ipPos, endIpPos - ipPos);
            
            size_t portPos = endIpPos + 1;
            size_t endPortPos = mensaje.find("/END", portPos);
            int serverPort = std::stoi(mensaje.substr(portPos, endPortPos - portPos));

            std::cout << "[DEBUG] Servidor " << serverIP << ":" << serverPort << " se ha desconectado" << std::endl;

            // Eliminar el servidor de la tabla de figuras usando clave compuesta IP:Puerto
            std::string serverKey = serverIP + ":" + std::to_string(serverPort);
            servidoresFiguras.erase(serverKey);
            
            // También eliminar las figuras asociadas a este servidor específico
            for (auto it = tablaDeFiguras.begin(); it != tablaDeFiguras.end();) {
                if (it->second.servidorIP == serverIP && it->second.servidorPuerto == serverPort) {
                    std::cout << "[DEBUG] Figura " << it->first << " eliminada de la tabla de ruteo" << std::endl;
                    it = tablaDeFiguras.erase(it); // Eliminar figura
                } else {
                    it++; // Avanzar al siguiente elemento
                }
            }
        }
    }
}

// Hilo para enviar broadcasts periódicos a todas las islas
void udpBroadcaster() {
    int conteo = 0;
    // Crear socket UDP para enviar mensajes de broadcast
    Socket udpSocket('d', false);
    udpSocket.bindSocketToInterface(NETWORK_INTERFACE);
    udpSocket.Bind(UDP_PORT_SEND);
    udpSocket.EnableUDP();

    std::string localIP = obtenerIP();
    std::string mensaje = "BEGIN/ON/TENEDOR/" + localIP + "/" + std::to_string(UDP_PORT_SEND) + "/END";

    std::cout << "[DEBUG] Enviando broadcasts a todas las islas" << std::endl;
    
    while (conteo < 2) {
        // Recorrer todas las direcciones de broadcast
        for (const auto& broadcastAddr : broadcastAddresses) {
            try {
                udpSocket.SendUDP(mensaje.c_str(), mensaje.size(), broadcastAddr.c_str(), UDP_PORT_SEND);
                std::cout << "[DEBUG] Broadcast enviado a " << broadcastAddr << ": " << mensaje << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ERROR] Error al enviar broadcast a " << broadcastAddr << ": " << e.what() << std::endl;
            }
            
            // Pequeña pausa entre envíos para no saturar la red
            usleep(100000); // 100ms
        }
        
        // Esperar antes del próximo ciclo de broadcasts
        sleep(5); // Cada 5 segundos
        conteo++;
    }
}

// Función principal
int main(int argc, char* argv[]) {
    signal(SIGINT, manejar_senal);
    
    // Inicializar el vector de direcciones de broadcast
    initBroadcastAddresses();

    std::cout << "[INFO] Total de direcciones de broadcast: " << broadcastAddresses.size() << std::endl;
    
    // Iniciar hilos
    std::thread udpListenerThread(udpListener);
    std::thread udpBroadcasterThread(udpBroadcaster);
    std::thread tcpHandlerThread(clienteHandler);
    
    // Esperar a que terminen los hilos (nunca deberían terminar)
    udpListenerThread.join();
    udpBroadcasterThread.join();
    tcpHandlerThread.join();

    return 0;
}