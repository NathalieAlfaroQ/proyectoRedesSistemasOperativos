/**
 *   UCR-ECCI
 *   CI-0123 Proyecto integrador de redes y sistemas operativos
 *
 *   Socket client/server example
 *
 *   Deben determinar la dirección IP del equipo donde van a correr el servidor
 *   para hacer la conexión en ese punto (ip addr)
 *
 **/

#include <stdio.h>
#include <cstring>
#include "Socket.h"

#define PORT 1234
#define BUFSIZE 512

int main( int argc, char ** argv ) {
   VSocket * s;
   char buffer[ BUFSIZE ];

   s = new Socket( 's', false);     // Creaite a new stream IPv4 socket
   memset( buffer, 0, BUFSIZE );	// Zero fill buffer

   /*
   En terminal no de VSC, poner: ip addr
   Tomar la eth0 en inet
   alfanath@Alfa:~$ ip addr
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc mq state UP group default qlen 1000
    link/ether 00:15:5d:f6:1e:ed brd ff:ff:ff:ff:ff:ff
    inet 172.23.148.148/20 brd 172.23.159.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::215:5dff:fef6:1eed/64 scope link
       valid_lft forever preferred_lft forever
   */

   s->MakeConnection( "172.23.148.148", PORT ); // Same port as server
   if ( argc > 1 ) {
      s->Write( argv[1] );		// If provided, send first program argument to server
   } else {
      s->Write( "Hello world 2025 ..." );
   }
   s->Read( buffer, BUFSIZE );	// Read answer sent back from server
   printf( "%s", buffer );	// Print received string, mirror example this will print same sent string

}

