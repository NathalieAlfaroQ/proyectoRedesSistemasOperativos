
 
// SSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>
#include <iostream>

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
SSLSocket::SSLSocket( bool IPv6 ) {

   this->InitVSocket( 's', IPv6 );

   this->SSLContext = nullptr;
   this->SSLStruct = nullptr;

   this->Init();					// Initializes to client context

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

   this->InitVSocket( id );

}


/**
  * Class destructor
  *
 **/
SSLSocket::~SSLSocket() {

// SSL destroy
   if ( nullptr != this->SSLContext ) {
      SSL_CTX_free( reinterpret_cast<SSL_CTX *>( this->SSLContext ) );
   }
   if ( nullptr != this->SSLStruct ) {
      SSL_free( reinterpret_cast<SSL *>( this->SSLStruct ) );
   }

   Close();

}


/**
  *  SSLInit
  *     use SSL_new with a defined context
  *
  *  Create a SSL object
  *
 **/
void SSLSocket::Init( bool serverContext ) {
   this->InitContext( serverContext );

   SSL * ssl = SSL_new( (SSL_CTX *) this->SSLContext );
   if (ssl == nullptr)
   {
      std::cout << "ssl null pointer" << std::endl;
   }
   
   this->SSLStruct = (void *) ssl;

}


/**
  *  InitContext
  *     use SSL_library_init, OpenSSL_add_all_algorithms, SSL_load_error_strings, TLS_server_method, SSL_CTX_new
  *
  *  Creates a new SSL server context to start encrypted comunications, this context is stored in class instance
  *
 **/
void SSLSocket::InitContext( bool serverContext ) {
   /*
   La versión real del protocolo utilizada se negociará hasta la versión más alta admitida mutuamente 
   por el cliente y el servidor. Los protocolos admitidos son SSLv3, TLSv1, TLSv1.1, TLSv1.2 y TLSv1.3.
   */
   const SSL_METHOD * method = TLS_client_method();
   if( method == nullptr) {
      std::cout << "Error: SSL_METHOD * method = TLS_client_method()" << std::endl;
   }
   /*
   El parámetro del método especifica si
   el contexto se utilizará para el lado del cliente o del servidor o ambos
   SSL_CTX_new() inicializa la lista de cifrados, la configuración de la caché de la sesión, las devoluciones de llamada, 
   las claves y certificados, y las opciones a sus valores predeterminados.
   Un cliente enviará mensajes de saludo del cliente TLSv1 e indicará que solo comprende TLSv1. 
   Un servidor solo entenderá los mensajes de saludo del cliente TLSv1. Esto significa especialmente que
   no entenderá los mensajes de saludo del cliente SSLv2 que se usan ampliamente por razones de compatibilidad; 
   */
   SSL_CTX * context = SSL_CTX_new( method );
   if (!context) {
      perror("Cant intialize context");
   }
   
   this->SSLContext = (void *) context;

   if ( serverContext ) {
      
   } else {
   }

   if ( nullptr == method ) {
      throw std::runtime_error( "SSLSocket::InitContext( bool )" );
   }

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
   // SSL_CTX * context = instance variable
   int st;

   if ( SSL_CTX_use_certificate_file( (SSL_CTX *) this->SSLContext, certFileName, SSL_FILETYPE_PEM ) <= 0 ) {	 // set the local certificate from CertFile
      st = SSL_get_error( (SSL *) this->SSLStruct, st );
      ERR_print_errors_fp( stderr );
      abort();
   }

   if ( SSL_CTX_use_PrivateKey_file( (SSL_CTX *) this->SSLContext, keyFileName, SSL_FILETYPE_PEM ) <= 0 ) {	// set the private key from KeyFile (may be the same as CertFile)
      st = SSL_get_error( (SSL *) this->SSLStruct, st );
      ERR_print_errors_fp( stderr );
      abort();
   }

   if ( ! SSL_CTX_check_private_key( (SSL_CTX *) this->SSLContext ) ) {	// verify private key
      st = SSL_get_error( (SSL *) this->SSLStruct, st );
      ERR_print_errors_fp( stderr );
      abort();
   }
}
 
void SSLSocket::SSLInitServerContext() {
   int error = -1;
   SSL_METHOD const *method;
   SSL_CTX *ctx;

   SSL_library_init();
   OpenSSL_add_all_algorithms();
   SSL_load_error_strings();

   method = TLS_server_method();  /* create new server-method instance */

   if (method == NULL) {
      perror("SSLSocket::SSLInitServerContext(): ");
   }
   
   ctx = SSL_CTX_new( method );   /* create new context from method */
   if ( ctx == NULL ) {
     ERR_print_errors_fp( stderr );
     abort();
   }

}

void SSLSocket::SSLInitServer(const char* certFileName, const char* keyFileName) {
   SSLInitServerContext();
   SSL* ssl = SSL_new((SSL_CTX*) this->SSLContext);
   if (ssl == NULL)
   {
      perror("SSLSocket::SSLInitServer(const char* certFileName, const char* keyFileName)");
   }

   this->SSLStruct = (void *) ssl;
   LoadCertificates(certFileName, keyFileName);
   
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
int SSLSocket::Connect( const char * hostName, int port ) {
   int st;
   int error = -1;
   st = this->DoConnect( hostName, port );		// Establish a non ssl connection first
      if (st == -1)
   {
      perror("SSLSocket::Connect( char *, int ) failed");
   }
   // The SSL_set_fd function assigns a socket to a Secure Sockets Layer (SSL) structure.
   error = SSL_set_fd((SSL *)this->SSLStruct, idSocket);
   if (error <= 0)
   {
      perror("SSL set fd failed");
   }
   
   // The SSL_connect function starts a Secure Sockets Layer (SSL) session with a remote server application.
   error = SSL_connect((SSL *)this->SSLStruct);
   if (error <= 0)
   {
      perror("SSL_connect failed");
   }

   return st;

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
int SSLSocket::Connect( const char * host, const char * service ) {
   int st;
   int error = -1;
   st = this->DoConnect( host, service );
   if (st == -1)
   {
      perror("SSLSocket::Connect( char *, char * ) failed");
   }
   
   // The SSL_set_fd function assigns a socket to a Secure Sockets Layer (SSL) structure.
   error = SSL_set_fd((SSL *)this->SSLStruct, idSocket);
   if (error <= 0)
   {
      perror("SSL set fd failed");
   }
   
   // The SSL_connect function starts a Secure Sockets Layer (SSL) session with a remote server application.
   error = SSL_connect((SSL *)this->SSLStruct);
   if (error <= 0)
   {
      perror("SSL_connect failed");
   }

   return st;

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
size_t SSLSocket::Read( void * buffer, size_t size ) {
   int st = SSL_read((SSL*) this->SSLStruct, buffer, size);

   if ( -1 == st ) {
      throw std::runtime_error( "SSLSocket::Read( void *, size_t )" );
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
size_t SSLSocket::Write( const char * string ) {
   int st = SSL_write( (SSL*) this->SSLStruct, string, strlen(string));
   std::cout << "Write 2: " << st << std::endl;
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
  *  Reads data from secure channel
  *
 **/
size_t SSLSocket::Write( const void * buffer, size_t size ) {

   int st = SSL_write( (SSL*) this->SSLStruct, buffer, size);
   std::cout << "Write 1: " << st << std::endl;
    if (st <= 0) {
        int error_code = SSL_get_error((SSL *)this->SSLStruct, st);
        const char *error_str = "Error desconocido";
        
        switch (error_code) {
            case SSL_ERROR_NONE:
                error_str = "SSL_ERROR_NONE";
                break;
            case SSL_ERROR_ZERO_RETURN:
                error_str = "SSL_ERROR_ZERO_RETURN";
                break;
            case SSL_ERROR_WANT_READ:
                error_str = "SSL_ERROR_WANT_READ";
                break;
            case SSL_ERROR_WANT_WRITE:
                error_str = "SSL_ERROR_WANT_WRITE";
                break;
            case SSL_ERROR_WANT_CONNECT:
                error_str = "SSL_ERROR_WANT_CONNECT";
                break;
            case SSL_ERROR_WANT_ACCEPT:
                error_str = "SSL_ERROR_WANT_ACCEPT";
                break;
            case SSL_ERROR_WANT_X509_LOOKUP:
                error_str = "SSL_ERROR_WANT_X509_LOOKUP";
                break;
            case SSL_ERROR_SYSCALL:
                error_str = "SSL_ERROR_SYSCALL";
                break;
            case SSL_ERROR_SSL:
                error_str = "SSL_ERROR_SSL";
                break;
        }
        
        std::cerr << "SSL_write error: " << error_str << " (Code: " << error_code << ")" << std::endl;
        
        throw std::runtime_error("SSLSocket::Write(const void *, size_t)");
    }
   return st;
}


/**
  * Accept method
  *    use base class Socket to create a new instance
  *
  *  @returns   a new class instance
  *
  *  Waits for a new connection to service (TCP mode: stream)
  *
 **/
SSLSocket * SSLSocket::Accept() {
   int id, st;
   SSLSocket * ssl;

   if ( 0 >= st ) {
      throw std::runtime_error( "SSLSocket::Accept()" );
   }

   return ssl;

}


/**
 *   Show SSL certificates
 *
 **/
void SSLSocket::ShowCerts() {
   X509 *cert;
   char *line;

   cert = SSL_get_peer_certificate( (SSL *) this->SSLStruct );		 // Get certificates (if available)
   if ( nullptr != cert ) {
      printf("Server certificates:\n");
      line = X509_NAME_oneline( X509_get_subject_name( cert ), 0, 0 );
      printf( "Subject: %s\n", line );
      free( line );
      line = X509_NAME_oneline( X509_get_issuer_name( cert ), 0, 0 );
      printf( "Issuer: %s\n", line );
      free( line );
      X509_free( cert );
   } else {
      printf( "No certificates.\n" );
   }

}


/**
 *   Return the name of the currently used cipher
 *
 **/
const char * SSLSocket::GetCipher() {

   return SSL_get_cipher( reinterpret_cast<SSL *>( this->SSLStruct ) );

}

