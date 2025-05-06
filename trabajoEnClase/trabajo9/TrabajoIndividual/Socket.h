/**
 *  Establece la definición de la clase Socket para efectuar la comunicación
 *  de procesos que no comparten memoria, utilizando un esquema de memoria
 *  distribuida.  El desarrollo de esta clase se hará en varias etapas, primero
 *  los métodos necesarios para los clientes, en la otras etapas los métodos para el servidor,
 *  manejo de IP-v6, conexiones seguras y otros
 *
 *  Universidad de Costa Rica
 *  ECCI
 *  CI0123 Proyecto integrador de redes y sistemas operativos
 *  2023-ii
 *  Grupo: 2 
 *
 **/

#ifndef Socket_h
#define Socket_h
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "VSocket.h"

class Socket : public VSocket {

   public:
      Socket( char, bool = false );
      Socket(int);
      ~Socket();
      void Close();
      int Connect( const char *, int );
      // int Connect( const char *, const char * );
      size_t Read( void *, size_t );
      size_t Write( const void *, size_t );
      size_t Write( const char * );

      Socket * SSLInit();
      void SSLInitServer( const char * certFileName, const char * keyFileName);
      void SSLCreate( Socket * original );
      void SSLAccept();
      const char * SSLGetCipher();
      Socket * Accept();
      int SSLConnect( const char * hostName, int port );
      size_t SSLWrite( const void * buffer, size_t size );
      size_t SSLRead( void * buffer, size_t size );
   // private:
      void SSLInitContext();
      void SSLLoadCertificates( const char * certFileName, const char * keyFileName );
      void SSLInitServerContext();
      void SSLShowCerts();

      void * SSLContext;					// SSL context
      void * SSLStruct;					// SSL BIO (Basic Input/Output)

};

#endif

