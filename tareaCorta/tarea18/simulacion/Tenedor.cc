// Encabezados
#include "Tenedor.h"

using namespace std;

/**
 *  Clase constructor
 */
Tenedor::Tenedor(Servidor *server) : servidor(server)
{
    pthread_mutex_init(&queueMutex, nullptr);
    // Se enciende el tenedor
   // cout << "BEGIN/ON/TENEDOR/END" << endl;
}

/**
 *  Clase destructor
 */
Tenedor::~Tenedor()
{
    pthread_mutex_destroy(&queueMutex);
}

/**
 * Inicia el hilo.
 */
void Tenedor::iniciar()
{
    pthread_create(&thread, nullptr, procesaSolicitud, this);
    cout << "BEGIN/ON/TENEDOR/END" << endl;
}

/**
 * Detiene.
 * Espera a que el hilo del servidor finalice.
 */
void Tenedor::detener()
{
    cout << "\n x Cliente - Tenedor x" << endl;
    corriendo = false;
    cout << "BEGIN/OFF/Tenedor/END" << endl;
    pthread_join(thread, nullptr);
}

/**
 *  Procesa las solicitudes entrantes en la cola de mensajes del servidor.
 *  Este método se ejecuta dentro del hilo del servidor y maneja las solicitudes de forma asincrónica.
 */
void *Tenedor::procesaSolicitud(void *arg)
{
    Tenedor *server = static_cast<Tenedor *>(arg);

    while (server->corriendo)
    {
        pthread_mutex_lock(&server->queueMutex);

        if (!server->messageQueue.empty())
        {
            Mensaje msg = server->messageQueue.front();
            server->messageQueue.pop();
            pthread_mutex_unlock(&server->queueMutex);

            // Solicita la figura a la cola
            // cout << "BEGIN/OBJECT/END" << endl;
            // cout << "BEGIN/OK/perro\ngato\npinguino\n/END" << endl;
            string contenido = server->servidor->obtenerFigura(msg.animal);
            server->recibeFigura(msg.animal, contenido);
        }
        else
        {
            pthread_mutex_unlock(&server->queueMutex);
        }

        sleep(1);
    }

    return nullptr;
}

/**
 *  Recibe una figura de la cola.
 */
void Tenedor::recibeFigura(const string &figura, const string &contenido)
{
    if (!contenido.empty())
    {
        figuraGuardada = contenido;
        cout << "\n Tenedor tiene la figura." << endl;
        cout << "\n BEGIN/GET/" << figura << "/END" << endl;
    }
    else
    {
        cout << "\n Tenedor no recibió la figura." << endl;
    }
}

/**
 * Muestra la figura solicitada.
 */
void Tenedor::mostrarFigura()
{
    if (!figuraGuardada.empty())
    {
        cout << "\n Cliente tiene la figura por el Tenedor: \n"
             << figuraGuardada << endl;
    }
    else
    {
      //  cout << "\n Tenedor no tiene la figura." << endl;
    }
}

/**
 * Envia la solicitud al servidor para que la procese.
 */
void Tenedor::enviarSolicitud(const Mensaje &msg)
{
    if (corriendo == false)
    {
        cout << "BEGIN/ERROR/101/No se puede establecer conexion con el tenedor/END" << endl;
        return;
    }

    // Simular que se apagó justo cuando se envia la solicitud
    bool simulacionDesconexion = false;

    if (simulacionDesconexion)
    {
        corriendo = false;
        cout << "BEGIN/ERROR/103/Sucedio una perdida abrupta de conexion al tenedor/END" << endl;
        return;
    }
    
    enviarConTimeout(msg);
}

/*
* Verifica si es posible conectarse, en cuyo cayo que no, hace el intento
* 3 veces y cada intento es de 5 segundos.
*/
void Tenedor::enviarConTimeout(const Mensaje &msg)
{
    int intentos = 0;
    const int maxIntentos = 3;
    const int timeoutSegundos = 5;
    bool exito = false;

    while (intentos < maxIntentos && !exito)
    {
        cout << "\n Intento " << (intentos + 1) << ": Tenedor solicita la figura al Servidor." << endl;

        pthread_mutex_lock(&queueMutex);
        messageQueue.push(msg);
        pthread_mutex_unlock(&queueMutex);

        auto inicio = chrono::steady_clock::now();

        while (chrono::steady_clock::now() - inicio < chrono::seconds(timeoutSegundos))
        {
            // Respuesta del servidor
            if (!figuraGuardada.empty())
            {
                exito = true;
                break;
            }

            this_thread::sleep_for(chrono::milliseconds(100));
        }

        if (!exito)
        {
            cout << "BEGIN/ERROR/106/Ha sucedido un timeout de solicitud al tenedor/END" << endl;
        }

        intentos++;
    }

    if (!exito)
    {
        cout << "BEGIN/ERROR/103/Sucedio una perdida abrupta de conexion al tenedor/END" << endl;
        corriendo = false;
    }
}