/*
* Aqui el cliente es cuando una figura especifica para
* que el servidor se la muestre, pero ademas puede
* observar la lista de las figuras disponibles y puede
* agregar una figura.
*/

// Bibliotecas
#include <iostream>
#include <cstring>
#include <string>

// Encabezados
#include "Socket.h"

// Puerto de comunicacion entre cliente-servidor
#define PORT 5013
// Tamano del buffer, para poder almacenar el mensaje en el buffer
#define BUFSIZE 4096

int main(int argc, char *argv[])
{
    // Verifica que el usuario escriba un comando. Si no, muestra cómo se usa.
    if (argc < 2)
    {
        //std::cerr << "[Uso]: " << argv[0] << " <list | add <figura> | <nombre_figura_sin_.txt>>\n";
        std::cerr << "BEGIN/ERROR/200/La solicitud del cliente tiene un formato inválido/END\r\n";
        return 1;
    }

    std::string comando = argv[1];  // Primer argumento como comando
    std::string host = "127.0.0.1"; // Dirección del servidor (cambiar)
    std::string request;            // Petición HTTP a enviar

    // Construye la solicitud HTTP dependiendo del comando recibido:list, add o nombre de figura:
    // Solicitud para listar todas las figuras
    if (comando == "list")
    {
        // Variable para almacenar esos detalles
        request = "GET /listado HTTP/1.1\r\n"
                  "Host: localhost\r\n"
                  "Connection: close\r\n"
                  "\r";
    }
    else if (comando == "add" && argc == 3)
    {
        // Solicitud para agregar una nueva figura al servidor
        std::string figura = argv[2];
        request = "GET /add?nombre=" + figura + " HTTP/1.1\r\n"
                                                "Host: localhost\r\n"
                                                "Connection: close\r\n"
                                                "\r";
    }
    else
    {
        // Solicitud para obtener una figura específica (usando su nombre)
        request = "GET /figura?nombre=" + comando + " HTTP/1.1\r\n"
                                                    "Host: localhost\r\n"
                                                    "Connection: close\r\n"
                                                    "\r";
    }

    try
    {
        Socket client('s');                        // Crea un socket tipo cliente
        client.MakeConnection(host.c_str(), PORT); // Conecta al servidor
        client.Write(request.c_str());             // Envía la petición HTTP al servidor

        // Búfer para almacenar respuesta del servidor
        char buffer[BUFSIZE]; 
        int bytesRead;

        std::cout << "\n[Respuesta del servidor]:\n\n";

        try
        {
            // Lee los datos recibidos gracias al servidor, en bloques hasta que no quede nada
            // y lo muestra
            do
            {
                memset(buffer, 0, BUFSIZE);               // Limpia el búfer
                bytesRead = client.Read(buffer, BUFSIZE); // Lee datos del socket

                if (bytesRead > 0)
                {
                    // Imprime datos
                    std::cout << std::string(buffer, bytesRead); 
                }
            } while (bytesRead > 0);
        }
        catch (std::exception &ex)
        {
            // Maneja errores de lectura (por ejemplo, cierre inesperado del servidor)
            std::string msg = ex.what();

            if (msg.find("conexión cerrada") == std::string::npos)
            {
                std::cerr << "BEGIN/ERROR/102/Sucedió una pérdida abrupta de conexión al servidor/END\r\n" << std::endl;
            }
        }

        // Cierra la conexión del socket
        client.Close(); 
    }
    catch (std::exception &e)
    {
        // Manejo de errores generales de conexión o escritura
        std::cerr << "Pérdida de conexión o error en los comandos del cliente" << e.what() << std::endl;
    }

    return 0;
}