# Cliente-Servidor de Figuras ASCII

## Descripción Servidor HTTP

Se implementa un sistema de almacenamiento y recuperación de dibujos en formato ASCII utilizando un servidor HTTP en C++. Los clientes pueden agregar, listar y solicitar dibujos desde el servidor mediante peticiones HTTP.

El sistema está dividido en varias partes bien estructuradas:

- **Servidor HTTP (HTTPServer.out)**: Atiende solicitudes HTTP de clientes para listar, agregar o recuperar figuras ASCII almacenadas.
- **Sistema de Archivos (FileSystem.bin)**: Simula un sistema de almacenamiento donde se guardan las figuras en bloques de 256 bytes.
- **Cliente HTTP (HTTPClient.out)**: Permite interactuar con el servidor mediante comandos para agregar, listar y visualizar figuras.
- **Servidor tradicional (Server.out)**: Versión inicial del servidor antes de implementar HTTP (se mantiene para referencia).
- **Cliente tradicional (Client.out)**: Cliente de consola tradicional para agregar figuras.

## Interacción entre componentes

- **HTTPClient** envía solicitudes HTTP al **HTTPServer**:
  - `/add?nombre=nombre_figura`: Agrega una figura desde la carpeta `Figuras/` al sistema de archivos.
  - `/listado`: Lista todas las figuras almacenadas.
  - `/figura?nombre=nombre_figura`: Solicita y recupera una figura específica.

- **HTTPServer** interpreta estas solicitudes, accede al **FileSystem.bin** para leer o escribir las figuras, y responde con una página HTML sencilla con el contenido solicitado.

- **Navegador Web**: También puede interactuar directamente con el servidor usando las mismas rutas HTTP.

## Estructura del Almacenamiento (FileSystem.bin)

- El sistema guarda las figuras en bloques de 256 bytes.
- Existencia de una lista de índices de directorios creados en el bloque 0.
- Los nombres de las figuras se almacenan junto con su ubicación en bloques para recuperación.
- Solo el servidor tiene acceso directo al archivo binario.

## Ejecución del Proyecto

### Compilar

Simplemente ejecutar:

                make

Esto generará:

- Server.out
- HTTPServer.out
- Client.out
- HTTPClient.out

### Limpiar el proyecto

                make clean

Esto elimina todos los ejecutables, objetos y el sistema de archivos (`FileSystem.bin`).

### Ejecutar el servidor HTTP

                ./HTTPServer.out

Esto deja el servidor escuchando peticiones en el puerto configurado (por defecto 5013).

### Usar el cliente HTTP (ejemplos)

Agregar una figura:

                ./HTTPClient.out add gato

Listar figuras:

                ./HTTPClient.out list

Ver una figura:

                ./HTTPClient.out gato

### Usar el navegador (opcional)

También se puede interactuar desde un navegador escribiendo la URL:

                http://numero_ip:5013/listado
                http://numero_ip:5013/add?nombre=gato
                http://numero_ip:5013/figura?nombre=gato.txt

### Ejecutar el servidor convencional

En una terminal:

        ./Server.out

Esto abrirá el servidor y quedará escuchando clientes.

### Correr el cliente

En otra terminal (o en otra ventana):

        ./Client.out

El cliente se conectará al servidor y mostrará el menú de comandos.

### Comandos del Cliente

Una vez conectado, se puede escribir:
`list`: Muestra todas las figuras almacenadas.
`get <nombre>`:Muestra una figura específica por nombre. Ejemplo: `get gato`
`add <nombre>`:Agrega una figura nueva desde un archivo de texto. Ejemplo: `add perro` |
`disconnect`:Termina la conexión con el servidor.
