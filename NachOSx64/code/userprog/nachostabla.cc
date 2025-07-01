#include "nachostabla.h"
#include "synch.h"
#include <iostream>

#define MAX_OPEN_FILES 32  // Máximo número de archivos abiertos por proceso

// Constructor: inicializa la tabla y el mapa de bits
NachosOpenFilesTable::NachosOpenFilesTable() {
    openFiles = new int[MAX_OPEN_FILES];          // Arreglo para guardar los descriptores UNIX
    openFilesMap = new BitMap(MAX_OPEN_FILES);    // Bitmap para controlar qué entradas están ocupadas
    usage = 0; 
    // Inicialmente no hay hilos usando la tabla
    tableLock = new Semaphore("OpenFilesTable Lock", 1);
    // Inicializamos todas las entradas como vacías
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        openFiles[i] = -1; // -1 indica que no hay ningún archivo en esa posición
    }
    // Reservar stdin, stdout, stderr
    openFilesMap->Mark(0);
    openFilesMap->Mark(1);
    openFilesMap->Mark(2);
}

// Destructor: libera memoria utilizada por la tabla
NachosOpenFilesTable::~NachosOpenFilesTable() {
    delete openFilesMap;
    delete[] openFiles;
}

// Abre un archivo registrando su descriptor UNIX en la tabla
// Devuelve un "NachosHandle" (índice en la tabla) o -1 si está llena
int NachosOpenFilesTable::Open(int unixHandle) {
    tableLock->P();  // Adquiere el semáforo para evitar condiciones de carrera
    int freeSlot = openFilesMap->Find(); // Busca un espacio libre en la tabla
    if (freeSlot == -1) {
        printf("Error: No hay espacio para abrir más archivos.\n");
        return -1;
    }
    openFiles[freeSlot] = unixHandle; // Guarda el descriptor UNIX
    tableLock->V(); // Libera el semáforo
    return freeSlot; // Retorna el índice como identificador Nachos
}

// Cierra un archivo dado un "NachosHandle"
// Devuelve 0 si se cerró con éxito o -1 en caso de error
int NachosOpenFilesTable::Close(int nachosHandle) {
    tableLock->P(); // Adquiere el semáforo para evitar condiciones de carrera
    if (!openFilesMap->Test(nachosHandle)) {
        tableLock->V();  // Libera el semáforo antes de retornar
        printf("Error: Intento de cerrar un archivo no abierto.\n");
        return -1;
    }
    int unixHandle = openFiles[nachosHandle];
    openFilesMap->Clear(nachosHandle); // Marca el espacio como libre en el bitmap
    openFiles[nachosHandle] = -1; // Limpia el valor en la tabla
    tableLock->V();  // Libera el semáforo
    return unixHandle; // Retorna el descriptor UNIX cerrado
}

// Verifica si un NachosHandle está actualmente en uso
bool NachosOpenFilesTable::isOpened(int nachosHandle) {
    return openFilesMap->Test(nachosHandle);
}

// Devuelve el descriptor UNIX asociado a un NachosHandle
int NachosOpenFilesTable::getUnixHandle(int nachosHandle) {
    if (!isOpened(nachosHandle)) {
        return -1; // Si no está en uso, retorna error
    }
    return openFiles[nachosHandle];
}

// Se llama cuando un hilo nuevo empieza a usar esta tabla
void NachosOpenFilesTable::addThread() {
    usage++; // Incrementa el contador de hilos activos
}

// Se llama cuando un hilo deja de usar esta tabla
// Si es el último hilo, se destruye la tabla
void NachosOpenFilesTable::delThread() {
    usage--;
    // Si el contador de uso es menor o igual a 0, se destruye la tabla
    if (usage > 0 && --usage == 0) {
        delete this;
    }
}

// Muestra el contenido de la tabla en consola
void NachosOpenFilesTable::Print() {
    std::cout << "Open Files Table:\n";
    for (int i = 0; i < openFilesMap->NumClear(); ++i) {
        if (openFilesMap->Test(i)) {
            std::cout << "NachosHandle: " << i
                      << " -> UnixHandle: " << openFiles[i] << '\n';
        }
    }
}