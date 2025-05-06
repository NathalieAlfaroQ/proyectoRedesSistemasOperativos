// Bibliotecas
#include <fstream>

// Encabezados
#include "Servidor.h"

using namespace std;

/**
 * Constructor
 * Se inicia el servidor de figuras
 * Se configura el mutex
 * Se lee una figura solicitada
 */
Servidor::Servidor(const string &figura)
{
    // Inicializar mutex
    pthread_mutex_init(&queueMutex, nullptr);
    ifstream archivo(figura + ".txt");

    if (archivo.is_open())
    {
        string contenido((istreambuf_iterator<char>(archivo)),
                         istreambuf_iterator<char>());
        figuras[figura] = contenido;
        archivo.close();

        cout << "BEGIN/ON/SERVIDOR/192.168.1.10/8080/END" << endl;
    }
    else
    {
        throw ios_base::failure("BEGIN/ERROR/201/El dibujo solicitado no fue encontrado/END");
    }
}

/**
 * Destructor, también destruye el mutex
 */
Servidor::~Servidor()
{
    pthread_mutex_destroy(&queueMutex);
}

/**
 * Inicia el hilo del servidor que procesa la solicitud
 */
void Servidor::iniciar()
{
    pthread_create(&thread, nullptr, procesaSolicitud, this);
}

/**
 * Se detiene el hilo del servidor.
 * Se detiene el servidor.
 * Se espera a que el hilo termine.
 */
void Servidor::detener()
{
    cout << "\n x Tenedor - Servidor x " << endl;
    corriendo = false;
    cout << "BEGIN/OFF/SERVIDOR/192.168.1.10/8080/END" << endl;
    pthread_join(thread, nullptr);
}

/**
 * Procesa solicitudes en la cola de mensajes del servidor.
 * Se ejecuta en el hilo del servidor.
 */
void *Servidor::procesaSolicitud(void *arg)
{
    Servidor *server = static_cast<Servidor *>(arg);

    while (server->corriendo)
    {
        pthread_mutex_lock(&server->queueMutex);

        if (!server->messageQueue.empty())
        {
            Mensaje msg = server->messageQueue.front();
            server->messageQueue.pop();

            pthread_mutex_unlock(&server->queueMutex);

            string contenido = server->obtenerFigura(msg.animal);
            cout << "Servidor procesa la solicitud. " << endl;
        }
        else
        {
            pthread_mutex_unlock(&server->queueMutex);
        }

        sleep(1);
    }

    return nullptr;
}

// Obtiene la figura solicitada.
string Servidor::obtenerFigura(const string &figura)
{
    if (figuras.empty())
    {
        cout << "BEGIN/ERROR/104/El servidor de dibujos se encuentra vacio/END" << endl;
        return "";
    }

    if (figuras.find(figura) != figuras.end())
    {
        cout << "\n Servidor envía la figura al Tenedor." << endl;
        return figuras[figura];
    }
    else
    {
        cout << "\n BEGIN/201/El dibujo solicitado no fue encontrado/END " << figura << endl;
        return "";
    }
}

// Envía la solicitud al Servidor
void Servidor::enviarSolicitud(const Mensaje &msg)
{
    if (corriendo == false)
    {
        cout << "BEGIN/ERROR/100/No se puede establecer conexion con el servidor/END" << endl;
        return;
    }

    // Simular que se apagó justo cuando iba a enviarse:
    // Cambiar esto a true para probar
    bool simulacionDesconexion = false;

    if (simulacionDesconexion)
    {
        // Simulamos que se apagó de golpe
        corriendo = false;
        cout << "BEGIN/ERROR/102/Sucedio una perdida abrupta de conexion al servidor/END" << endl;
        return;
    }

    pthread_mutex_lock(&queueMutex);
    messageQueue.push(msg);
    pthread_mutex_unlock(&queueMutex);
}