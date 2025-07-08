/*
 * Servidor que atiende las solicitudes del cliente,
 * para darle la figura que solicita.
 */

// Bibliotecas
#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <csignal> 

// Encabezados
#include "Socket.h"
#include "FileSystem.h"

// Puerto de comunicacion entre cliente-servidor
#define PORT 5013
// Tamano del buffer, para poder almacenar el mensaje en el buffer
#define BUFSIZE 1024

Socket* servidor = nullptr;

// Función para obtener la IP de la PC con rol de servidor (como "eth0" o "wlan0")
std::string obtenerIP()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;

    std::string ip_final = "127.0.0.1"; // Valor por defecto

    getifaddrs(&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next)
    {

        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET &&
            std::string(ifa->ifa_name) != "lo")
        {
            sa = (struct sockaddr_in *)ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            ip_final = std::string(addr);
            break;
        }
    }
    freeifaddrs(ifap);
    return ip_final;
}

/*
* Esta función toma la señal de control C como indicativo que el servidor
* se debe apagar ya o que lo apaga, es aquí donde se notifica al usuario
* que el servidor se apaga, porque al inicio da una ilusión de un servidor
* que siempre estará en ejecución, pero realmente este se puede apagar 
* cuando se da control C yasí esta acción nos permite liberar recursos,
* porque si lo dejamos en ejecución como un servidor web para siempre,
* ahí no hay cómo liberar los recursos.
*/
void manejar_senal(int signo) {
    if (signo == SIGINT) {
        // Se indica que el servidor esta apagado
        std::string ip_local = obtenerIP();
        std::cout << std::endl;
        std::string response = "HTTP/1.1 BEGIN/OFF/SERVIDOR/" + ip_local + "/" + std::to_string(PORT) + "/END\r\n";
        std::cout << response << std::endl;

        if (servidor) {
            delete servidor;
            servidor = nullptr;
        }

        // Termina el programa
        exit(0);  
    }
}

// Funcion del servidor cunado llega un cliente
void task(VSocket *client)
{
    FileSystem fs("FileSystem.bin");
    char buffer[BUFSIZE];
    memset(buffer, 0, BUFSIZE);

    try
    {
        // Recibe lo que el cliente escribe
        client->Read(buffer, BUFSIZE);
        // Lee el mensaje HTTP
        std::string request(buffer);
        std::cout << "\n ==== Procesando ====\n" << request << "\n";

        std::istringstream reqStream(request);
        std::string method, fullPath, protocol;
        /*
         * Extrae los datos de la solicitud.
         *
         * method -> lo que el cliente quiere hacer: GET
         * fullPath ->  parte del servidor que se requiere: figuras
         * protocol -> tipo de mensaje, HTTP 1.1
         */
        reqStream >> method >> fullPath >> protocol;

        if (method != "GET")
        {
            client->Write("HTTP/1.1 BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END\r\n");
            client->Close();
            delete client;
            return;
        }

        size_t qmark = fullPath.find('?');
        std::string path = fullPath.substr(0, qmark);
        std::string query = (qmark != std::string::npos) ? fullPath.substr(qmark + 1) : "";

        std::cout << "Ruta: " << path << "\n";
        std::cout << "Consulta: " << query << "\n";

        auto construirRespuestaHTML = [](const std::string &contenido)
        {
            std::string cuerpo = "<html><head><meta charset=\"utf-8\"></head><body><pre>\n" + contenido + "\n</pre></body></html>";
            std::stringstream ss;
            ss << "Content-Type: text/html; charset=utf-8\r\n"
               << "Content-Length: " << cuerpo.size() << "\r\n"
               << "\r\n"
               << cuerpo;
            return ss.str();
        };

        /*
         * El cliente solicita una figura, se buscar ese archivo de texto en el
         * en el sistema de archivos.
         * Si no se encuentra se lanza un error 404, sino muestra la figura en
         * HTML.
         */
        if (path == "/figura")
        {
            size_t pos = query.find("nombre=");

            if (pos != std::string::npos)
            {
                std::string nombre = query.substr(pos + 7);

                if (nombre.find(".txt") == std::string::npos)
                    nombre += ".txt";

                std::string figura = fs.Leer_Figura(nombre);

                if (figura.find("[INFO] Figura") != std::string::npos)
                {
                    std::string response = "HTTP/1.1 BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END\r\n";
                    client->Write(response.c_str(), response.size());
                }
                else
                {
                    figura = "HTTP/1.1 BEGIN/OK/END\n\n" + figura;
                    std::string respuesta = construirRespuestaHTML(figura);
                    client->Write(respuesta.c_str(), respuesta.size());
                }
            }
            else
            {
                std::string error = "HTTP/1.1 BEGIN/ERROR/200/La solicitud del cliente tiene un formato inválido/END\r\n";
                client->Write(error.c_str(), error.size());
            }
        }
        else if (path == "/listado")
        {
            // Da una lista de todos los dibujos disponibles
            std::string lista = fs.Listar_Figuras();
            std::string respuesta = construirRespuestaHTML(lista);
            client->Write(respuesta.c_str(), respuesta.size());
        }
        else if (path == "/add")
        {
            // Para agregar una figura al sistema con el nombre que da el cliente
            size_t pos = query.find("nombre=");

            if (pos != std::string::npos)
            {
                std::string nombre = query.substr(pos + 7);

                if (nombre.find(".txt") == std::string::npos)
                {
                    nombre += ".txt";
                }

                if (fs.Agregar_Figura(nombre))
                {
                    std::string response = "Figura agregada\n";
                    client->Write(response.c_str(), response.size());
                }
                else
                {
                    std::string response = "HTTP/1.1 BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END\r\n";
                    client->Write(response.c_str(), response.size());
                }
            }
            else
            {
                // Si la ruta/figura no existe, se manda un error
                std::string error = "HTTP/1.1 BEGIN/ERROR/200/La solicitud del cliente tiene un formato inválido/END\r\n";
                client->Write(error.c_str(), error.size());
            }
        }
        else
        {
            std::cout << "Comando no reconocido: " << path << "\n";
            std::string error = "HTTP/1.1 BEGIN/ERROR/200/La solicitud del cliente tiene un formato inválido/END\r\n";
            client->Write(error.c_str(), error.size());
        }
    }
    catch (std::exception &ex)
    {
        std::cerr << " " << ex.what() << "\n";
        std::string error = "HTTP/1.1 BEGIN/ERROR/102/Sucedió una pérdida abrupta de conexión al servidor/END\r\n";
        client->Write(error.c_str(), error.size());
    }

    // Se cierra la comunicacion con el cliente
    client->Close();
    delete client;
}

int main(int argc, char **argv)
{
    signal(SIGINT, manejar_senal);

    FileSystem fs("FileSystem.bin");
    fs.Crear_FileSystem();

    //Socket *servidor = new Socket('s');
    servidor = new Socket('s');
    servidor->Bind(PORT);
    servidor->MarkPassive(5);
    // Se indica que el servidor esta encendido
    std::string ip_local = obtenerIP();
    std::string response = "HTTP/1.1 BEGIN/ON/SERVIDOR/" + ip_local + "/" + std::to_string(PORT) + "/END\r\n";
    std::cout << response << std::endl;
    std::cout << "Figuras que puede cargar al sistema:\nbobEsponja\ndragon\nfeliz\ngato\npajaro\nperro\npez\npinguino\ntriste\nwhatsapp\n" << std::endl;

    while (true)
    {
        VSocket *cliente = servidor->AcceptConnection();
        // Cada se atiende en su propio hilo, asi el servidor
        // puede hablar con varios a la vez
        std::thread worker(task, cliente);
        worker.detach();
    }

    return 0;
}