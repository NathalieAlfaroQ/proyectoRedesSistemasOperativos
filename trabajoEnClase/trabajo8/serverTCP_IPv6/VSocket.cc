/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2025-i
  *  Grupos: 1 y 3
  *
  ****** VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.h"


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::BuildSocket( char t, bool IPv6 ){

   this->IPv6 = IPv6;

   if(this->IPv6) {
       if(t == 'd'){
          this->idSocket = socket(AF_INET6, SOCK_DGRAM,0);
       } else {
          this->idSocket = socket(AF_INET6, SOCK_STREAM,0);	
       }
    } else {
       if(t == 's'){
          this->idSocket = socket(AF_INET, SOCK_STREAM,0);
       } else {
          this->idSocket = socket(AF_INET, SOCK_DGRAM,0);
       }
    }


   // In case it is done incorrectly, print an error message.
	if((idSocket) == -1){
      throw std::runtime_error( "VSocket::BuildSocket, (reason)" );
	}
}


/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {

   this->Close();

}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close(){
   int connection;
   // If the socket is open, close it
   if (this->idSocket != -1) {
     connection = close(this->idSocket);
     if (connection == -1) {
      throw std::runtime_error( "VSocket::Close()" );
     }
     this->idSocket = -1;
   }

}


/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::EstablishConnection( const char * hostip, int port ) {

   int connection;

   // Creates a connection based on whether it is an IPv6 or not
   if(this->IPv6) {
     // Struct that holds the server information
     struct sockaddr_in6 server_address6;
     server_address6.sin6_flowinfo  = 0;
     // Makes it IPv6
     server_address6.sin6_family    = AF_INET6;
     server_address6.sin6_port      = htons(port);
     // Passes the address into binary
     if (inet_pton(AF_INET6, hostip, &server_address6.sin6_addr) <= 0) {
       throw std::runtime_error("inet_pton for IPv6");
     }
     // Attempts to create connection
     connection = connect(idSocket, (struct sockaddr *) &server_address6, sizeof(server_address6));
   } else {
     // Same as the if above, except for IPv4
     struct sockaddr_in server_address;
     server_address.sin_family   = AF_INET;
     server_address.sin_port     = htons(port);
     if (inet_pton(AF_INET, hostip, &server_address.sin_addr) <= 0) {
         throw std::runtime_error("inet_pton for IPv4");
     }
     connection = connect(idSocket, (struct sockaddr *) &server_address, sizeof(server_address));
   }
 
   // Send an error if done incorrectly.
   if(-1 == connection) {
      throw std::runtime_error( "VSocket::EstablishConnection" );
   }
 
   return connection;


}


/**
  * EstablishConnection method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::EstablishConnection( const char *host, const char *service ) {
   int st;
  struct addrinfo hints, *result, *rp;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Stream socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  st = getaddrinfo( host, service, &hints, &result );

  for ( rp = result; rp; rp = rp->ai_next ) {
    st = connect( idSocket, rp->ai_addr, rp->ai_addrlen );
    if ( 0 == st )
        break;
  }

  freeaddrinfo( result );
  return st;

}


/**
  * Bind method
  *    use "bind" Unix system call (man 3 bind) (server mode)
  *
  * @param      int port: bind a unamed socket to a port defined in sockaddr structure
  *
  *  Links the calling process to a service at port
  *
 **/
int VSocket::Bind( int port ) {
   int st = -1;

   // Creates a connection based on whether it is an IPv6 or not
   if(this->IPv6) {
     // Struct that holds the server information
     struct sockaddr_in6 server_address6;
     // Makes it IPv6
     server_address6.sin6_family = AF_INET6;
     server_address6.sin6_addr = in6addr_any;
     server_address6.sin6_port = htons(port);
     // Attempts to bind
     st = bind(idSocket, (struct sockaddr *) &server_address6, sizeof(server_address6));
   } else {
     // Same as the if above, except for IPv4
     struct sockaddr_in server_address;
     server_address.sin_family = AF_INET;
     server_address.sin_addr.s_addr = INADDR_ANY;
     server_address.sin_port = htons(port);
     st = bind(idSocket, (struct sockaddr *) &server_address, sizeof(server_address));
   }
 
   // Send an error if done incorrectly.
   if(-1 == st) {
      throw std::runtime_error( "VSocket::Bind2025" );
   }
 
   return st;
}


/**
  * MarkPassive method
  *    use "listen" Unix system call (man listen) (server mode)
  *
  * @param      int backlog: defines the maximum length to which the queue of pending connections for this socket may grow
  *
  *  Establish socket queue length
  *
 **/
int VSocket::MarkPassive( int backlog ) {
   int st = -1;

   st = listen(idSocket, backlog);

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::MarkPassive" );
   }

   return st;

}


/**
  * WaitForConnection method
  *    use "accept" Unix system call (man 3 accept) (server mode)
  *
  *
  *  Waits for a peer connections, return a sockfd of the connecting peer
  *
 **/
int VSocket::WaitForConnection( void ) {
   int st = -1;

 st = accept(idSocket, nullptr, nullptr);
  if ( -1 == st ) {
   throw std::runtime_error( "VSocket::WaitForConnection" );
  }

   return st;

}


/**
  * Shutdown method
  *    use "shutdown" Unix system call (man 3 shutdown) (server mode)
  *
  *
  *  cause all or part of a full-duplex connection on the socket associated with the file descriptor socket to be shut down
  *
 **/
int VSocket::Shutdown( int mode ) {
   int st = -1;
   st = shutdown(idSocket, mode);

   if ( -1 == st ) {
   throw std::runtime_error( "VSocket::Shutdown" );
   }

   return st;

}


// UDP methods 2025

/**
  *  sendTo method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to send data
  *
  *  Send data to another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::sendTo( const void * buffer, size_t size, void * addr ) {
   int st = -1;
   st = sendto(this->idSocket, buffer, size, 0, (sockaddr*)addr, sizeof(struct sockaddr_in));

   if (st == -1) {
     throw std::runtime_error("VSocket::sendTo");
   }

   return st;

}


/**
  *  recvFrom method
  *
  *  @param	const void * buffer: data to send
  *  @param	size_t size data size to send
  *  @param	void * addr address to receive from data
  *
  *  @return	size_t bytes received
  *
  *  Receive data from another network point (addr) without connection (Datagram)
  *
 **/
size_t VSocket::recvFrom( void * buffer, size_t size, void * addr ) {
   int st = -1;
 socklen_t addr_len = sizeof(struct sockaddr_in);

  st = recvfrom(this->idSocket, buffer, size, 0, (struct sockaddr*) addr, &addr_len);
  if (st == -1) {
    throw std::runtime_error("VSocket::recvFrom");
  }
   return st;

}

