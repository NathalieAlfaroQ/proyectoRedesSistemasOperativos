#include <iostream>
#include <cstring>
#include <string>
#include "Socket.h"

#define PORT 5015    // Puerto al que se conectará el cliente al tenedor
#define BUFSIZE 512  // Tamaño del búfer para lectura de datos

int main(int argc, char* argv[]) {
    // Verifica si hay al menos 2 argumentos adicionales
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <IP_servidor> <comando>\n";
        return 1;
    }

    std::string serverIP = argv[1];
    std::string comando = argv[2];     // Primer argumento como comando
    std::string request;               // Petición HTTP a enviar

    // Construye la solicitud HTTP dependiendo del comando recibido
    if (comando == "list") {
        request = "GET /listado HTTP/1.1\r\n\r\n";
    } else {
        request = "GET /figura=" + comando + " HTTP/1.1\r\n\r\n";
    }

    Socket client('s');                     // Crea un socket tipo cliente
    client.MakeConnection(serverIP.c_str(), PORT);  // Conecta al servidor
    client.Write(request.c_str());           // Envía la petición HTTP

    // Leer la respuesta completa
    char buffer[BUFSIZE];
    int totalBytesRead = 0;
    int bytesRead;
    
    std::cout << "[Respuesta del tenedor]:\n\n";
    
    // Bucle de lectura
    while (true) {
        try {
            memset(buffer, 0, BUFSIZE);
            bytesRead = client.Read(buffer, BUFSIZE - 1);
            
            if (bytesRead <= 0) {
                break; // No hay más datos para leer
            }
            
            buffer[bytesRead] = '\0';
            std::cout << buffer;
            totalBytesRead += bytesRead;
        } 
        catch (const std::runtime_error& e) {
            // Si la excepción es por cierre de conexión, terminamos normalmente
            if (strstr(e.what(), "conexión cerrada") != nullptr) {
                break;
            } else {
                // Para otros errores, los propagamos
                throw;
            }
        }
    }
    
    // Cerrar la conexión de manera ordenada
    client.Close();
    
    return 0;
}