#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>

const int TAM_BLOQUE = 256;
const std::string ARCHIVO = "almacenamiento.bin";

void leer_figura(const std::string& nombre_figura) {
    std::ifstream archivo(ARCHIVO, std::ios::binary);
    if (!archivo) {
        std::cerr << "No se pudo abrir el archivo binario." << std::endl;
        return;
    }

    std::vector<char> directorio(TAM_BLOQUE);
    archivo.read(directorio.data(), TAM_BLOQUE);

    uint8_t contador = static_cast<uint8_t>(directorio[TAM_BLOQUE - 1]);
    std::cout << "Figuras en directorio: " << (int)contador << "\n\n";

    for (int i = 0; i < contador; ++i) {
        int offset = 2 + i * 16;

        char nombre_raw[15] = {0};
        std::memcpy(nombre_raw, &directorio[offset], 14);
        std::string nombre(nombre_raw);
        nombre.erase(nombre.find_last_not_of("\0 ") + 1);

        uint8_t inicio = static_cast<uint8_t>(directorio[offset + 14]);
        uint8_t final = static_cast<uint8_t>(directorio[offset + 15]);

        std::cout << "Figura " << i << ": '" << nombre << "' (inicio: " << (int)inicio << ", final: " << (int)final << ")" << std::endl;

        if (nombre == nombre_figura) {
            int byte_inicio = TAM_BLOQUE + (inicio - 1) * TAM_BLOQUE + 1;
            int byte_final  = TAM_BLOQUE + final * TAM_BLOQUE;
            int longitud = byte_final - byte_inicio;

            archivo.seekg(byte_inicio, std::ios::beg);
            std::vector<char> figura_bytes(longitud);
            archivo.read(figura_bytes.data(), longitud);

            std::string figura(figura_bytes.data(), figura_bytes.size());
            figura.erase(figura.find_last_not_of('\0') + 1);

            std::cout << "\nFigura encontrada:\n";
            std::cout << "Nombre: " << nombre << std::endl;
            std::cout << "Bloque de inicio: " << (int)inicio << std::endl;
            std::cout << "Bloque de final: " << (int)final << std::endl;
            std::cout << "Contenido:\n\n" << figura << std::endl;
            return;
        }
    }

    std::cout << "\nFigura '" << nombre_figura << "' no encontrada." << std::endl;
}

int main() {
    leer_figura("perro");
    leer_figura("cafe");
    leer_figura("pez");
    return 0;
}