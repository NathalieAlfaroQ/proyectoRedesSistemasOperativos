/**
 *
 *   UCR-ECCI
 *
 *   IPv4 TCP client normal or SSL according to parameters
 *
 **/

#include <stdio.h>
#include <string.h>

#include "VSocket.h"
#include "Socket.h"
#include "SSLSocket.h"
#include <cstdlib>
#include <iostream>

#define	MAXBUF	4096

/**
**/
int main( int argc, char * argv[] ) {
   int request_option = std::atoi(argv[1]); // Convierte el argumento a entero
   VSocket * client;
   int st, port = 80;
   char response[ MAXBUF ];
   char * os = (char *) "163.178.104.62";
   char * request;
   // Seleccionar el request basado en la opción
   switch (request_option) {
      case 1:
         request = (char *) "GET /aArt/index.php?disk=Disk-01&fig=rabbit.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";
         break;      
      case 2:
         
         request = (char *) "GET /aArt/index.php?disk=Disk-01&fig=whale-1.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";
         break;
      case 3:
         request = (char *) "GET /aArt/index.php?disk=Disk-01&fig=seahorse.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r\n";
         break;

      case 4:
         request = (char *) "GET /aArt/index.php?disk=Disk-01&fig=error.txt\r\nHTTP/v1.1\r\nhost: redes.ecci\r\n\r";
         break;
      default:
          printf("Opción de request no válida.");
          return -1;
  }
   if (argc > 1 ) {
      port = 443;
      client = new SSLSocket();	// Create a new stream socket for IPv4
   } else {
      client = new Socket( 's' );
      port = 80;
   }

   memset( response, 0 , MAXBUF );
   client->MakeConnection( os, port );
   client->Write(  (char * ) request, strlen( request ) );
   st = client->Read( response, MAXBUF );

   char *startTag = strstr(response, "<PRE>");
   char *endTag  = strstr(response, "</PRE>");

   if (startTag && endTag) {
      startTag += 5;  // Skip the "<PRE>" tag
      int length = endTag - startTag;

      char asciiArt[1024];
      strncpy(asciiArt, startTag, length);
      asciiArt[length] = '\0';

      printf("ASCII Art found:\n%s\n", asciiArt);
  } else {
      printf("No ASCII art found in the response.\n");
  }
}

