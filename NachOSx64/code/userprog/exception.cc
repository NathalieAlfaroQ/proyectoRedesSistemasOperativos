// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "nachostabla.h"
#include "addrspace.h"
#include <stdint.h>

extern NachosOpenFilesTable *OpenFileTable; // Tabla de archivos abiertos
void NachosForkThread(void *p);  // Prototipo de función para manejar el fork de un hilo

/*
 * System call interface: returnFromSystemCall()
 * Called when a system call is finished.  The registers are updated
 * Código tomado de la asistencia de los syscall
 */
void returnFromSystemCall() {

        machine->WriteRegister( PrevPCReg, machine->ReadRegister( PCReg ) );		// PrevPC <- PC
        machine->WriteRegister( PCReg, machine->ReadRegister( NextPCReg ) );			// PC <- NextPC
        machine->WriteRegister( NextPCReg, machine->ReadRegister( NextPCReg ) + 4 );	// NextPC <- NextPC + 4

}       // returnFromSystemCall

/*
 *  System call interface: Halt()
 */
void NachOS_Halt() {		// System call 0

	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();

}


/*
 *  System call interface: void Exit( int )
 */
void NachOS_Exit() {		// System call 1
   int exitStatus = machine->ReadRegister(4);
   
   DEBUG('a', "Exit, iniciado por el usuario.\n");
   DEBUG('a', "Estado de Exit: %d\n", exitStatus);
   
   currentThread->Finish();	// Current thread is finishing
}


/*
 *  System call interface: SpaceId Exec( char * )
 */
void NachOS_Exec() {		// System call 2
}


/*
 *  System call interface: int Join( SpaceId )
 */
void NachOS_Join() {		// System call 3
}


/*
 *  System call interface: void Create( char * )
 */
void NachOS_Create() {		// System call 4
}


/*
 *  System call interface: OpenFileId Open( char * )
 */
void NachOS_Open() {		// System call 5
   // Obtener la dirección en memoria del usuario donde empieza el nombre del archivo
   int addr = machine->ReadRegister(4);  // Registro $a0 en MIPS contiene el puntero al nombre del archivo

   // Leemos el nombre del archivo desde memoria del usuario, byte por byte
   char filename[256];  // Buffer local donde almacenaremos el nombre del archivo
   int i = 0;           // Índice para recorrer los bytes
   int byte;            // Variable temporal para almacenar el byte leído

   // Leemos hasta 255 caracteres o hasta encontrar el fin de cadena ('\0')
   while (i < 255) {
      if (!machine->ReadMem(addr + i, 1, &byte)) {  // Leer 1 byte desde la memoria de usuario
         // Si la lectura falla, reportamos error y salimos
         printf("[Error leyendo nombre del archivo en NachOS_Open]\n");
         machine->WriteRegister(2, -1);  // Retornamos -1 al usuario indicando error
         returnFromSystemCall();        // Restauramos el PC y salimos del syscall
         return;
      }

      filename[i] = (char)byte;   // Convertimos el entero leído a carácter
      if (filename[i] == '\0')    // Si encontramos fin de cadena, salimos del ciclo
         break;
      i++;
   }
   filename[255] = '\0';  // Aseguramos que el string esté bien terminado

   // Abrimos el archivo usando la función del sistema
   int unixHandle = OpenForReadWrite(filename, true);  // 'true' indica que se crea el archivo si no existe

   // Si la apertura falla, devolvemos error
   if (unixHandle == -1) {
      printf("[Error abriendo archivo en NachOS_Open: %s]\n", filename);
      machine->WriteRegister(2, -1);  // Retornamos -1 al usuario
      returnFromSystemCall();        // Salimos del syscall
      return;
   }

   // Registramos el descriptor de archivo UNIX en la tabla de archivos abiertos
   int nachosHandle = OpenFileTable->Open(unixHandle);

   // Si no hay espacio en la tabla de archivos, devolvemos error
   if (nachosHandle == -1) {
      printf("[Error: tabla de archivos NachOS llena]\n");
      close(unixHandle);            // Cerramos el archivo UNIX porque no se pudo registrar
      machine->WriteRegister(2, -1);  // Retornamos -1 al usuario
      returnFromSystemCall();
      return;
   }

   // Devolvemos al programa usuario el descriptor de archivo válido en NachOS
   machine->WriteRegister(2, nachosHandle);  // Registro descriptor de archivo

   // Restauramos el contador de programa para continuar con la ejecución normal del usuario
   returnFromSystemCall();
}


/*
 *  System call interface: OpenFileId Write( char *, int, OpenFileId )
 */
void NachOS_Write() {		// System call 6
   // Se obtienen los argumentos del syscall desde los registros
   int addr = machine->ReadRegister(4);  // Dirección del buffer en la memoria del usuario
   int size = machine->ReadRegister(5);  // Número de bytes a escribir
   int fd = machine->ReadRegister(6);    // Descriptor del archivo

   // Verificamos que el tamaño a escribir sea válido
   if (size <= 0) {
      machine->WriteRegister(2, 0);  // Nada que escribir, devolvemos 0
      returnFromSystemCall();
      return;
   }

   // Reservamos memoria local para almacenar el contenido del buffer de usuario
   char* buffer = new char[size];

   // Leemos el contenido de la memoria del usuario y lo copiamos al buffer local
   for (int i = 0; i < size; ++i) {
      int byte;
      if (!machine->ReadMem(addr + i, 1, &byte)) {  // Lee byte por byte desde memoria de usuario
         printf("[Error leyendo memoria del usuario en NachOS_Write]\n");
         delete[] buffer;                          // Liberamos memoria
         machine->WriteRegister(2, -1);            // Indicamos error en el registro de retorno
         returnFromSystemCall();
         return;
      }
      buffer[i] = (char)byte;                      // Guardamos el byte leído en el buffer
   }

   int written = -1;  // Variable para almacenar la cantidad de bytes escritos

   // Console->P();
   // Selección según el descriptor de archivo
   switch (fd) {
      case ConsoleInput:
         // No se permite escribir en la entrada estándar
         printf("[Error: no se puede escribir en ConsoleInput (fd 0)]\n");
         written = -1;
         break;

      case ConsoleOutput:
         // Escribimos el contenido del buffer a la salida estándar (stdout)
         fwrite(buffer, 1, size, stdout);
         fflush(stdout);        // Aseguramos que se imprima inmediatamente
         written = size;        // Indicamos que se escribieron todos los bytes
         break;

      case ConsoleError:
         // En ConsoleError se espera imprimir un entero (usando valor en el registro 4)
         printf("%d\n", machine->ReadRegister(4));
         written = sizeof(int);  // Devuelve tamaño de entero como bytes escritos
         break;

      default:
         // Archivos comunes: verificamos si el descriptor está abierto
         if (!OpenFileTable->isOpened(fd)) {
               printf("[Error: descriptor de archivo %d no está abierto]\n", fd);
               written = -1;  // Indicamos error
         } else {
               // Obtenemos el descriptor UNIX correspondiente al archivo abierto
               int unixHandle = OpenFileTable->getUnixHandle(fd);

               // Escribimos al archivo usando la syscall write del sistema
               written = write(unixHandle, buffer, size);

               if (written < 0) {
                  // Error al escribir en el archivo
                  printf("[Error al escribir en archivo fd %d]\n", fd);
                  written = -1;
               }
         }
         break;
   }
   // Console->V();

   // Escribimos el resultado de la operación en el registro de retorno (r2)
   machine->WriteRegister(2, written);

   // Liberamos el buffer usado para copiar datos del usuario
   delete[] buffer;

   // Volvemos al programa de usuario
   returnFromSystemCall();
}


/*
 *  System call interface: OpenFileId Read( char *, int, OpenFileId )
 */
void NachOS_Read() {		// System call 7
   // Leer los argumentos de los registros del sistema
   int bufferPointer = machine->ReadRegister(4);  // Dirección del buffer en la memoria del usuario
   int size = machine->ReadRegister(5);           // Número máximo de bytes a leer
   int socketId = machine->ReadRegister(6);       // Descriptor de archivo/socket

   // Crear un buffer temporal en espacio del kernel para almacenar los datos leídos
   char *buffer = new char[size];

   int bytesRead = 0;  // Variable para guardar cuántos bytes fueron realmente leídos

   // Leer datos desde el archivo o socket. Esto se hace con la syscall de UNIX `read`
   // que usa el descriptor `socketId`, y escribe en `buffer`, hasta `size` bytes
   bytesRead = ::read(socketId, buffer, size);

   // Verificar si ocurrió un error durante la lectura
   if (bytesRead < 0) {
      perror("Error reading");  // Mostrar un mensaje de error en consola (kernel)
      machine->WriteRegister(2, -1);  // Escribir -1 en el registro 2 para indicar fallo
      delete[] buffer;  // Liberar el buffer temporal
      returnFromSystemCall();  // Regresar al programa de usuario
      return;
   }

   // Transferir los datos leídos desde el buffer del kernel hacia la memoria del usuario
   for (int i = 0; i < bytesRead; ++i) {
      // `WriteMem` escribe un byte a la vez en la memoria del programa usuario
      if (!machine->WriteMem(bufferPointer + i, 1, buffer[i])) {
         // Si falla al escribir en memoria de usuario, reportar error y abortar
         printf("Error writing to user memory.\n");
         machine->WriteRegister(2, -1);  // Indicar error en el resultado
         delete[] buffer;  // Liberar el buffer del kernel
         returnFromSystemCall();
         return;
      }
   }

   // Liberar el buffer temporal del kernel ya que no se necesita más
   delete[] buffer;

   // Escribir en el registro 2 el número real de bytes que se lograron leer
   machine->WriteRegister(2, bytesRead);

   // Finalizar la llamada al sistema y volver al programa de usuario
   returnFromSystemCall();
}


/*
 *  System call interface: void Close( OpenFileId )
 */
void NachOS_Close() {		// System call 8
   // Leer desde el registro 4 el "handle" (descriptor) de archivo usado por el programa de usuario
   int nachosHandle = machine->ReadRegister(4);

   // Llamar a la tabla de archivos abiertos del hilo actual para cerrar el archivo/socket
   // Esto traduce el descriptor de NachOS a uno del sistema UNIX
   int unixHandle = OpenFileTable->Close(nachosHandle);

   // Si `Close` devolvió -1, hubo un error cerrando el descriptor (por ejemplo, no existe)
   if (unixHandle == -1) {
      printf("\nCLOSE: Error closing NachOS handle:%d", nachosHandle);
   } else {
      // Si todo salió bien, cerrar el descriptor real del sistema (UNIX)
      ::close(unixHandle);
   }

   // Finalizar la llamada al sistema y volver al programa de usuario
   returnFromSystemCall();
}


/*
 *  System call interface: void Fork( void (*func)() )
 */
void NachOS_Fork() {		// System call 9
   DEBUG('u', "Entering Fork System call\n");

   // Crear un nuevo hilo para ejecutar la función fork
   Thread *newT = new Thread("child fork");
   // Asignar el espacio de direcciones del hilo actual al nuevo hilo
   newT->space = new AddrSpace(currentThread->space);
   // Leer syscall a ejecutar
   long funcAddr = (long)machine->ReadRegister(4);

   // Crear el hilo con la nueva funcion auxiliar
   newT->Fork(NachosForkThread, (void *)funcAddr);

   returnFromSystemCall(); 
   DEBUG('u', "Exiting Fork System call\n");
}

 /*
 *  Auxiliary function for Fork
 *  This function is executed in the new thread created by Fork.
 */
void NachosForkThread(void *p) {
   AddrSpace *space = currentThread->space;
   space->InitRegisters();  // Registros limpios
   space->RestoreState();  // Restaurar page table del hilo actual

   int funcAddr = (int)(long)p;  // Dirección de la función a ejecutar
   machine->WriteRegister(PCReg, funcAddr);
   machine->WriteRegister(NextPCReg, funcAddr + 4);
   machine->WriteRegister(RetAddrReg, 4);  // Retorno a Exit

   machine->Run();  // Salta a código de usuario
   ASSERT(false);
}


/*
 *  System call interface: void Yield()
 */
void NachOS_Yield() {		// System call 10
}


/*
 *  System call interface: Sem_t SemCreate( int )
 */
void NachOS_SemCreate() {		// System call 11
}


/*
 *  System call interface: int SemDestroy( Sem_t )
 */
void NachOS_SemDestroy() {		// System call 12
}


/*
 *  System call interface: int SemSignal( Sem_t )
 */
void NachOS_SemSignal() {		// System call 13
}


/*
 *  System call interface: int SemWait( Sem_t )
 */
void NachOS_SemWait() {		// System call 14
}


/*
 *  System call interface: Lock_t LockCreate( int )
 */
void NachOS_LockCreate() {		// System call 15
}


/*
 *  System call interface: int LockDestroy( Lock_t )
 */
void NachOS_LockDestroy() {		// System call 16
}


/*
 *  System call interface: int LockAcquire( Lock_t )
 */
void NachOS_LockAcquire() {		// System call 17
}


/*
 *  System call interface: int LockRelease( Lock_t )
 */
void NachOS_LockRelease() {		// System call 18
}


/*
 *  System call interface: Cond_t LockCreate( int )
 */
void NachOS_CondCreate() {		// System call 19
}


/*
 *  System call interface: int CondDestroy( Cond_t )
 */
void NachOS_CondDestroy() {		// System call 20
}


/*
 *  System call interface: int CondSignal( Cond_t )
 */
void NachOS_CondSignal() {		// System call 21
}


/*
 *  System call interface: int CondWait( Cond_t )
 */
void NachOS_CondWait() {		// System call 22
}


/*
 *  System call interface: int CondBroadcast( Cond_t )
 */
void NachOS_CondBroadcast() {		// System call 23
}


/*
 *  System call interface: Socket_t Socket( int, int )
 */
void NachOS_Socket() {			// System call 30
   // Leer el parámetro "domain" desde el registro 4 (ej: AF_INET_NachOS)
   int domain = machine->ReadRegister(4);

   // Traducir constantes propias de NachOS a las de Unix
   if (domain == AF_INET_NachOS) {
      domain = AF_INET;  // IPv4
   } else if (domain == AF_INET6_NachOS) {
      domain = AF_INET6;  // IPv6
   } else {
      // Dominio inválido
      printf("NachOS_Socket, invalid domain\n");
      returnFromSystemCall();  // Finalizar la syscall sin hacer nada
   }

   // Leer el parámetro "socket type" desde el registro 5 (ej: SOCK_STREAM_NachOS)
   int sockType = machine->ReadRegister(5);

   // Traducir constantes propias de NachOS a las de Unix
   if (sockType == SOCK_STREAM_NachOS) {
      sockType = SOCK_STREAM;  // TCP
   } else if (sockType == SOCK_DGRAM_NachOS) {
      sockType = SOCK_DGRAM;   // UDP
   } else {
      // Tipo de socket inválido
      printf("NachOS_Socket, invalid socket type.\n");
      returnFromSystemCall();  // Finalizar la syscall
   }

   // Crear el socket usando la llamada real del sistema Unix
   int sockID = socket(domain, sockType, 0);

   // Verificar si la creación fue exitosa
   if (sockID > 0) {
      printf("Socket se creo correctamente.\n");
   } else {
      printf("NachOS_Socket, could not create socket.\n");
   }

   // Variable para almacenar el handle NachOS que asocia el socket creado
   int NachOSHandle;

   // Registrar el socket en la tabla de archivos abiertos de NachOS
   // y obtener el handle correspondiente
   NachOSHandle = OpenFileTable->Open(sockID);

   // Esta línea obtiene el descriptor Unix de vuelta desde la tabla,
   // int check = OpenFileTable->getUnixHandle(NachOSHandle);
   // printf("NachOS_Socket, NachOS a Unix: %d\n", check); // Útil para debug

   // Escribir el handle NachOS resultante en el registro 2 (valor de retorno)
   machine->WriteRegister(2, NachOSHandle);

   // Finalizar la llamada al sistema
   returnFromSystemCall();
}


/*
 *  System call interface: Socket_t Connect( char *, int )
 */
void NachOS_Connect() {		// System call 31
   // Leer el identificador del socket desde el registro 4
   int socketId = machine->ReadRegister(4);

   // Leer el puerto de destino desde el registro 6
   int port = machine->ReadRegister(6);

   // Leer el puntero hacia la cadena de texto con la IP (en memoria de usuario) desde el registro 5
   int ipPointer = machine->ReadRegister(5);

   // Esta pregunta tuya: "// como pasa del const char a int?" → 
   // Aquí el "const char*" de C se pasa como un puntero a memoria de usuario, que es representado por un entero (la dirección base de la cadena en memoria NachOS).

   // Crear un buffer local para almacenar la dirección IP (ej: "192.168.0.1")
   char ip[16];  // 15 caracteres + terminador nulo '\0'
   memset(ip, 0, sizeof(ip));  // Inicializar el buffer a ceros

   // Leer byte por byte la dirección IP desde la memoria del usuario NachOS
   printf("Reading IP address from user memory...\n");
   for (int i = 0; i < 15; ++i) {  // Leer hasta 15 caracteres como máximo
      int byte;

      // Leer un byte desde la dirección ipPointer + i
      if (!machine->ReadMem(ipPointer + i, 1, &byte)) {
         printf("Error reading byte %d from IP address.\n", i);
         machine->WriteRegister(2, -1);  // Retornar error si falla lectura
         return;
      }

      // Guardar el byte leído en el buffer IP (conversión a char)
      ip[i] = static_cast<char>(byte);

      // Podrías activar esta línea si quisieras detener la lectura al encontrar fin de cadena
      // if (byte == '\0') break;
   }

   // Mostrar la IP leída desde memoria de usuario (debug)
   printf("IP read: %s\n", ip);

   // Configurar estructura sockaddr_in con los datos del servidor
   struct sockaddr_in serverAddr;
   memset(&serverAddr, 0, sizeof(serverAddr));  // Inicializar en cero
   serverAddr.sin_family = AF_INET;             // Tipo de red: IPv4
   serverAddr.sin_port = htons(port);           // Convertir el puerto a formato de red (big endian)

   // Convertir la IP del formato texto a binario (red)
   if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
      // Si la IP es inválida, se muestra un mensaje de error
      printf("Invalid IP address: %s\n", ip);
      machine->WriteRegister(2, -1);  // Retornar error
      return;
   }

   // Llamar a la función connect() de Unix para conectar el socket
   int result = connect(socketId, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
   
   // Verificar si la conexión fue exitosa
   if (result < 0) {
      // Error al conectar
      perror("Connection failed");  // Imprimir mensaje del sistema
      machine->WriteRegister(2, -1);  // Retornar error
   } else {
      // Conexión exitosa
      printf("Connected to %s:%d\n", ip, port);  // Mensaje de éxito
      machine->WriteRegister(2, 0);  // Retornar 0 como código de éxito
   }

   // Finalizar la llamada al sistema
   returnFromSystemCall();
}


/*
 *  System call interface: int Bind( Socket_t, int )
 */
void NachOS_Bind() {		// System call 32
}


/*
 *  System call interface: int Listen( Socket_t, int )
 */
void NachOS_Listen() {		// System call 33
}


/*
 *  System call interface: int Accept( Socket_t )
 */
void NachOS_Accept() {		// System call 34
}


/*
 *  System call interface: int Shutdown( Socket_t, int )
 */
void NachOS_Shutdown() {	// System call 25
}


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch ( which ) {

       case SyscallException:
          switch ( type ) {
             case SC_Halt:		// System call # 0
                NachOS_Halt();
                break;
             case SC_Exit:		// System call # 1
                NachOS_Exit();
                break;
             case SC_Exec:		// System call # 2
                NachOS_Exec();
                break;
             case SC_Join:		// System call # 3
                NachOS_Join();
                break;

             case SC_Create:		// System call # 4
                NachOS_Create();
                break;
             case SC_Open:		// System call # 5
                NachOS_Open();
                break;
             case SC_Read:		// System call # 6
                NachOS_Read();
                break;
             case SC_Write:		// System call # 7
                NachOS_Write();
                break;
             case SC_Close:		// System call # 8
                NachOS_Close();
                break;

             case SC_Fork:		// System call # 9
                NachOS_Fork();
                break;
             case SC_Yield:		// System call # 10
                NachOS_Yield();
                break;

             case SC_SemCreate:         // System call # 11
                NachOS_SemCreate();
                break;
             case SC_SemDestroy:        // System call # 12
                NachOS_SemDestroy();
                break;
             case SC_SemSignal:         // System call # 13
                NachOS_SemSignal();
                break;
             case SC_SemWait:           // System call # 14
                NachOS_SemWait();
                break;

             case SC_LckCreate:         // System call # 15
                NachOS_LockCreate();
                break;
             case SC_LckDestroy:        // System call # 16
                NachOS_LockDestroy();
                break;
             case SC_LckAcquire:         // System call # 17
                NachOS_LockAcquire();
                break;
             case SC_LckRelease:           // System call # 18
                NachOS_LockRelease();
                break;

             case SC_CondCreate:         // System call # 19
                NachOS_CondCreate();
                break;
             case SC_CondDestroy:        // System call # 20
                NachOS_CondDestroy();
                break;
             case SC_CondSignal:         // System call # 21
                NachOS_CondSignal();
                break;
             case SC_CondWait:           // System call # 22
                NachOS_CondWait();
                break;
             case SC_CondBroadcast:           // System call # 23
                NachOS_CondBroadcast();
                break;

             case SC_Socket:	// System call # 30
		NachOS_Socket();
               break;
             case SC_Connect:	// System call # 31
		NachOS_Connect();
               break;
             case SC_Bind:	// System call # 32
		NachOS_Bind();
               break;
             case SC_Listen:	// System call # 33
		NachOS_Listen();
               break;
             case SC_Accept:	// System call # 32
		NachOS_Accept();
               break;
             case SC_Shutdown:	// System call # 33
		NachOS_Shutdown();
               break;

             default:
                printf("Unexpected syscall exception %d\n", type );
                ASSERT( false );
                break;
          }
          break;

       case PageFaultException: {
          break;
       }

       case ReadOnlyException:
          printf( "Read Only exception (%d)\n", which );
          ASSERT( false );
          break;

       case BusErrorException:
          printf( "Bus error exception (%d)\n", which );
          ASSERT( false );
          break;

       case AddressErrorException:
          printf( "Address error exception (%d)\n", which );
          currentThread->Finish(); // finaliza el hilo que hizo la violación
          ASSERT(false);
          break;

       case OverflowException:
          printf( "Overflow exception (%d)\n", which );
          ASSERT( false );
          break;

       case IllegalInstrException:
          printf( "Ilegal instruction exception (%d)\n", which );
          ASSERT( false );
          break;

       default:
          printf( "Unexpected exception %d\n", which );
          ASSERT( false );
          break;
    }

}