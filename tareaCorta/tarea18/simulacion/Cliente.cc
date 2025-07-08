// Encabezado
#include "Cliente.h"

using namespace std;

// Constructor
// Puntero a Tenedor que gestiona las solicitudes
Cliente::Cliente(Tenedor *server) : tenedor(server) {}

// Envía la solicitud al Tenedor para tener la figura
void Cliente::solicitaFigura(const string &animal)
{
    // Simulamos que se perdió la conexión con el tenedor
    // true para probar
    bool simulacionPerdidaConexion = false;

    if (simulacionPerdidaConexion || tenedor == nullptr)
    {
        cout << "BEGIN/ERROR/202/Ha sucedido una perdida abrupta de conexion al cliente/END" << endl;
        return;
    }

    cout << "\n Enviando solicitud del Cliente al Tenedor." << endl;
    Mensaje msg;
    msg.animal = animal;
    tenedor->enviarSolicitud(msg);
}

// Muestra la figura guardada
void Cliente::mostrarFigura()
{
    tenedor->mostrarFigura();
}