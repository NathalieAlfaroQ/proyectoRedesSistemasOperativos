# Objetivo

Completar sus clases "VSocket" y "Socket" en C++ para que funcione con SSL en IPv4.


# Tareas

- Agregar una nueva clase "SSLSocket" para crear conexiones seguras.

- Se facilita el ejemplo "ipv4-ssl-cli.c" para analizar el funcionamiento de un programa que hace una conexión SSL.

- Se facilita la interfaz "SSLSocket.h".  Para esta etapa deben completar:
    
    - El método para conectarse de manera segura SSLConnect (SSL_connect)

    - El método Read, para lectura de datos por el canal SSL (SSL_read)

    - El método Write, para la escritura de datos por el canal SSL (SSL_write)

- Instancias nuevas de la clase:

    - void * SSLContext;	// SSL context
    - void * SSLStruct;	// SSL BIO basis input output

- El ejemplo "ipv4-ssl-cli.cc" debe funcionar correctamente


# Descripción detallada

Clase SSLSocket

        void SSLSocket::InitContext( boolean serverContext ); // We must create a method to define our context
            const SSL_Method * method = TLS_client_method(); // Check for errors
            SSL_CTX * context = SSL_CTX_new( method );  // Check for errors
            this->SSLContext = (void *) context;

        void SSLSocket::Init( boolean serverContext );	// Create a SSL socket, a new context must be created before
            this->InitContext();
            SSL * ssl = SSL_new( (SSL_CTX *) this->context );
            // Check for errors
            this->SSLStruct = (void *) ssl;

        int SSLSocket::Connect( const char *, int );
            MakeConnection( host, port );	// Establish a non SSL connection first
            // Call SSL_set_fd
            // Call SSL_connect()
            // check for errors

        int SSLSocket::Connect( char *, char * );
            // Similar as above

        int SSLSocket::Read( void *, int );
            // Use SSL_read

        int SSLSocket::Write( void *, int );
            // Use SSL_write  
    

# Ubuntu

Debe instalar "libssl-dev" para las bibliotecas SSL.

	
# Referencias

https://os.ecci.ucr.ac.cr/ci0123/Asignaciones/SocketsCourse.ppt

https://www.openssl.org/docs/man3.0/man7/ssl.html

El manual del sistema operativo Unix para los otros llamados al sistema.


# Compilar y ejecutar

      make
    ./ipv4-ssl-cli.out