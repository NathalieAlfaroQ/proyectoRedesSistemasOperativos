#ifndef NACHOSTABLA_H
#define NACHOSTABLA_H

#include "bitmap.h"

class Semaphore;

class NachosOpenFilesTable {
  public:
    NachosOpenFilesTable();						// Initialize
    ~NachosOpenFilesTable();						// De-allocate
    
    int Open( int UnixHandle );					// Register the file handle
    int Close( int NachosHandle );					// Unregister the file handle
    bool isOpened( int NachosHandle );
    int getUnixHandle( int NachosHandle );
    void addThread();							// If a user thread is using this table, add it
    void delThread();							// If a user thread is using this table, delete it

    void Print();								// Print contents
    
  private:
    int * openFiles;							// A vector with user opened files
    BitMap * openFilesMap;						// A bitmap to control our vector
    int usage;									// How many threads are using this table
    Semaphore *tableLock;
};

#endif // NACHOSTABLA_H