/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  *  Socket class implementation
  *
  * (Fedora version)
  *
 **/
 
// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>

#include "SSLSocket.h"
#include "Socket.h"

/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket(bool IPv6) {
   this->BuildSocket('s', IPv6);  // Crea socket tipo stream
   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;
   this->Init(false);  // Inicializa como cliente SSL
}


/**
  *  Class constructor
  *     use base class
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool IPv6: if we need a IPv6 socket
  *
 **/
SSLSocket::SSLSocket( char * certFileName, char * keyFileName, bool IPv6 ) {
}


/**
  *  Class constructor
  *
  *  @param     int id: socket descriptor
  *
 **/
SSLSocket::SSLSocket( int id ) {

   this->BuildSocket( id );

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {
   if (this->SSLStruct != nullptr) {
       SSL_free(reinterpret_cast<SSL*>(this->SSLStruct));
       this->SSLStruct = nullptr;
   }

   if (this->SSLContext != nullptr) {
       SSL_CTX_free(reinterpret_cast<SSL_CTX*>(this->SSLContext));
       this->SSLContext = nullptr;
   }

   this->Close();
}


/**
  *  SSLInit
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::Init(bool serverContext) {
   this->InitContext(serverContext);

   SSL_CTX* ctx = reinterpret_cast<SSL_CTX*>(this->SSLContext);
   SSL* ssl = SSL_new(ctx);

   if (!ssl) {
       throw std::runtime_error("SSLSocket::Init SSL");
   }

   SSL_set_fd(ssl, this->idSocket);  // Asocia el socket al canal SSL
   this->SSLStruct = ssl;
}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext(bool serverContext) {
   const SSL_METHOD* method;

   if (serverContext) {
       method = TLS_server_method();  // No se usa en este trabajo
   } else {
       method = TLS_client_method();  // Cliente moderno
   }

   if (!method) {
       throw std::runtime_error("SSLSocket::InitContext - Método inválido");
   }

   SSL_CTX* ctx = SSL_CTX_new(method);
   if (!ctx) {
       throw std::runtime_error("SSLSocket::InitContext - No se pudo crear contexto SSL");
   }

   this->SSLContext = ctx;
}


/**
 *  Load certificates
 *    verify and load certificates
 *
 *  @param	const char * certFileName, file containing certificate
 *  @param	const char * keyFileName, file containing keys
 *
 **/
 void SSLSocket::LoadCertificates( const char * certFileName, const char * keyFileName ) {
}
 

/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	int port, service number
 *
 **/
// Realiza conexión segura a host y puerto
int SSLSocket::MakeConnection(const char* host, int port) {
   // Primero hace la conexión TCP normal
   this->EstablishConnection(host, port);

   // Luego realiza el handshake SSL
   SSL* ssl = reinterpret_cast<SSL*>(this->SSLStruct);
   int st = SSL_connect(ssl);

   if (st != 1) {
       int err = SSL_get_error(ssl, st);
       fprintf(stderr, "SSL_connect failed with code %d\n", err);
       throw std::runtime_error("SSLSocket::MakeConnection - SSL_connect falló");
   }

   return 0;
}


/**
 *  Connect
 *     use SSL_connect to establish a secure conection
 *
 *  Create a SSL connection
 *
 *  @param	char * hostName, host name
 *  @param	char * service, service name
 *
 **/
int SSLSocket::MakeConnection(const char* host, const char* service) {
   throw std::runtime_error("SSLSocket::MakeConnection(host, service) no implementado");
}


/**
  *  Read
  *     use SSL_read to read data from an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity read
  *
  *  Reads data from secure channel
  *
 **/
// Leer datos cifrados
size_t SSLSocket::Read(void* buffer, size_t size) {
   SSL* ssl = reinterpret_cast<SSL*>(this->SSLStruct);
   int st = SSL_read(ssl, buffer, static_cast<int>(size));

   if (st < 0) {
       int err = SSL_get_error(ssl, st);
       fprintf(stderr, "SSL_read failed with code %d\n", err);
       throw std::runtime_error("SSLSocket::Read(void*, size_t)");
   }

   return st;
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Writes data to a secure channel
  *
 **/
size_t SSLSocket::Write(const char* text) {
   return Write(reinterpret_cast<const void*>(text), strlen(text));
}


/**
  *  Write
  *     use SSL_write to write data to an encrypted channel
  *
  *  @param	void * buffer to store data read
  *  @param	size_t size, buffer's capacity
  *
  *  @return	size_t byte quantity written
  *
  *  Reads data from secure channel
  *
 **/
// Escribir datos cifrados
size_t SSLSocket::Write(const void* buffer, size_t size) {
   SSL* ssl = reinterpret_cast<SSL*>(this->SSLStruct);
   int st = SSL_write(ssl, buffer, static_cast<int>(size));
   if (st <= 0) {
       int err = SSL_get_error(ssl, st);
       fprintf(stderr, "SSL_write failed with code %d\n", err);
       throw std::runtime_error("SSLSocket::Write(void*, size_t)");
   }
   return st;
}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   SSL* ssl = reinterpret_cast<SSL*>(this->SSLStruct);
   X509* cert = SSL_get_peer_certificate(ssl);
   if (cert) {
       printf("Server certificates:\n");
       char* subj = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
       printf("Subject: %s\n", subj);
       free(subj);
       char* issuer = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
       printf("Issuer: %s\n", issuer);
       free(issuer);
       X509_free(cert);
   } else {
       printf("No certificates.\n");
   }
}


/**
 *   Return the name of the currently used cipher
 *
 **/
const char* SSLSocket::GetCipher() {
   SSL* ssl = reinterpret_cast<SSL*>(this->SSLStruct);
   return SSL_get_cipher(ssl);
}

