# Cliente-Servidor de Figuras ASCII


## Descripción Servidor HTTP

Se implementa un sistema de almacenamiento y recuperación de dibujos en formato ASCII utilizando un servidor HTTP en C++. Los clientes pueden agregar, listar y solicitar dibujos desde el servidor mediante peticiones HTTP.

El sistema está dividido en varias partes:

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

- **HTTPServer** interpreta estas solicitudes, accede al **FileSystem.bin** para leer o escribir las figuras y responde con una página HTML con el contenido solicitado.

- **Navegador Web**: También puede interactuar directamente con el servidor usando las mismas rutas HTTP, Sin embargo no funciona en todos los navegadores.


## Estructura del Almacenamiento (FileSystem.bin)

- El sistema guarda las figuras en bloques de 256 bytes.
- Existencia de una lista de índices de directorios creados en el bloque 0.
- Los nombres de las figuras se almacenan junto con su ubicación en bloques para recuperación.
- Solo el servidor tiene acceso directo al archivo binario.


## Ejecución del Proyecto

### Compilar:

En la terminal, posicionarse en el directorio ` /TerceraEtapa/servidorConProtocolo `.

Limpiar el proyecto con ` make clean `, esto elimina todos los ejecutables, objetos y el sistema de archivos (`FileSystem.bin`).

Compilar con ` make `.

Esto generará:

- Server.out
- HTTPServer.out
- Client.out
- HTTPClient.out

### Ejecutar el servidor HTTP:

Aquí el servidor se queda escuchando solicitudes, para ejecutarlo se usa el comando ` ./HTTPServer.out `.

### Usar el cliente HTTP:

En una terminal se tiene corriendo el servidor, por lo cual se necesita otra terminal para usar comandos para el cliente, algunos comandos son:

- Agregar una figura al listado, por ejemplo:

                ./HTTPClient.out add pez

- Revisar el listado de figuras que he agregado:

                ./HTTPClient.out list

- Visualizar la figura ASCII de:

                ./HTTPClient.out gato

Si desea visualizar una figura, antes debe agregarla al listado para posteriormente imprimirla.

### Con el navegador:

También se puede interactuar desde un navegador escribiendo la URL:

                http://numero_ip:5013/listado
                http://numero_ip:5013/add?nombre=gato
                http://numero_ip:5013/figura?nombre=gato.txt

### Ejecutar el servidor convencional:

En una terminal colocar ` ./Server.out `. Esto abrirá el servidor y quedará escuchando clientes.

### Correr el cliente convencional:

En otra terminal (o en otra ventana), ejecutar el cliente con ` ./Client.out `. El cliente se conectará al servidor y mostrará el menú de comandos.

### Comandos del cliente convencional

Una vez conectado, se puede escribir:

- `list`: Muestra todas las figuras almacenadas.
- `get <nombre>`: Muestra una figura específica por nombre. Ejemplo: `get gato`.
- `add <nombre>`: Agrega una figura nueva desde un archivo de texto. Ejemplo: `add perro`.
- `disconnect`: Termina la conexión con el servidor.


## Pruebas de fugas de memoria con Valgrind 

- Primeramente asegurarse de tener instalado Valgrind, con los siguientes comandos para WSL:Ubuntu o Ubuntu: 

```bash
sudo apt update
sudo apt install valgrind
```

- Del archivo que se quiere observar si tiene fugas o no, hay que centrase en su ejecutable:

```bash
valgrind --leak-check=full ./HTTPClient.out

valgrind --leak-check=full ./HTTPServer.out
```

Para ver detalladamente Valgrind, se usa el comando:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./HTTPClient.out

valgrind --leak-check=full --show-leak-kinds=all ./HTTPServer.out
```

En este caso a los archivos que se les hace pruebas de Valgrind son al HTTPClient.cc y al HTTPServer.cc, pero se debe hacer exactamente con sus ejecutables porque solo asi lo soporta Valgrind.

Para el cliente en HTTP se observa que no hay fugas de memoria:

![Prueba de Valgrind para el Cliente HTTP](/TerceraEtapa/servidorConProtocolo/imagenes/valgrindCliente.png)

Para el servidor en HTTP se observa que no hay fugas de memoria:

![Prueba de Valgrind para el Servidor HTTP](/TerceraEtapa/servidorConProtocolo/imagenes/valgrindServer.png)