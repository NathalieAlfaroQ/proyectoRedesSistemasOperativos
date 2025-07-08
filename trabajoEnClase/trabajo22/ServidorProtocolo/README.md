# Compilación y Ejecución del Sistema de Tenedor-Figuras

Este README explica cómo compilar y ejecutar el sistema de comunicación basado en el protocolo colaborativo con el Servidor Tenedor y el Servidor de Figuras.

## Compilación

Primeramente, como el cliente se conecta por la red pública al tenedor, entre el tenedor y el servidor de figuras, deben comunicarse por la red privada, por lo que debe de cambiar el parametro "#define NETWORK_INTERFACE "interfaz_ip_privada" ", tanto en el tenedor, como en el servidor de figuras.

Abre una terminal en el directorio `ServidorProtocolo` y ejecuta los siguientes comandos en una terminal para compilar los programas:

        make

## Ejecución

En una terminal ejecute el siguiente comando

        ./ServidorTenedor.out

En otra terminal ejecute el siguente comando

        ./ServidorTenedor.out

En otra terminal ejecute el siguiente comando

        ./HTTPCliente.out <ip_tenedor> <listado> | <nombre_figura_sin.txt>

Si desea utilizar el cliente en un browser, haga lo siguiente

- Abra un browser (de preferencia Firefox, ya que con Edge, Chrome, etc no funcionará de la mejor manera al ser http y no https).

- Ejecute la siguiente instrucción en el buscador:

        http://ip_tenedor:5015/listado | figura=nombre_figura_sin.txt
