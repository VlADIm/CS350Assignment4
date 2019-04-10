#include <fstream>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>

//In the psuedocode P means wait and V means signal
int readcount=0, writecount=0;                   //(initial value = 0)
sem_t rmutex, wmutex, readTry, resource; //(initial value = 1)
int numWrites;

struct Node{
    int data;
    Node* next;
};

Node* head, tail;
void* readList(void * inp){
    sem_wait(&readTry);                 //Indicate a reader is trying to enter
//<ENTRY Section>
//  rmutex.P();                  //lock entry section to avoid race condition with other readers
//  readcount++;                 //report yourself as a reader
//  if (readcount == 1)          //checks if you are first reader
//    resource.P();              //if you are first reader, lock  the resource
//    rmutex.V();                  //release entry section for other readers
//      readTry.V();                 //indicate you are done trying to access the resource
//
//      <CRITICAL Section>
//      //reading is performed
//
//      <EXIT Section>
//        rmutex.P();                  //reserve exit section - avoids race condition with readers
//          readcount--;                 //indicate you're leaving
//            if (readcount == 0)          //checks if you are last reader leaving
//                resource.V();              //if last, you must release the locked resource
//                  rmutex.V();                  //release exit section for other readers
//            }
}

void* writeList(void* id1){
    int id = *(int*) id1;

    sem_wait(&wmutex); // Says we are writing 
    writecount++; 
    if (writecount == 1) // Checks if you're first writer
        sem_wait(&readTry); // Lock out the readers if we want to write 
    sem_post(&wmutex); //release entry section
                            //<CRITICAL Section>
    sem_wait(&resource); //reserve the resource for yourself - prevents other writers from simultaneously editing the shared resource
                        //writing is performed
    // TODO implement writing here in a way such that it doesnt cause problems

    sem_post(&resource); //release file
//
//                          <EXIT Section>
    sem_wait(&wmutex); //reserve exit section
    writecount--; //indicate you're leaving
    if (writecount == 0) //checks if you're the last writer
        sem_post(&readTry); //if you're last writer, you must unlock the readers. Allows them to try enter CS for reading
    sem_post(&wmutex); //release exit section
}
 

int main(int argc, char* argv[]){
    int numReaders=0, numWriters=0;
    Node* head = new Node{};
    if(argc != 4){
        //NO ARGS
    }
    else{
        //All the args
        int temp =std::stoi(argv[1]);
        if(temp < 1 || temp > 100){
            std::cout << "Arg 1 must be between 1 and 100" << std::endl;
            exit(1);

            // ERROR
        }
        numWrites = temp;
        temp = std::stoi(argv[2]);
        if(temp < 1 || temp > 9){
            std::cout << "Arg 2 must be between 1 and 9" << std::endl;
            exit(1);
            // ERROR
        }
        numReaders = temp;
        temp =std::stoi(argv[3]);
        if(temp < 1 || temp > 9){
            std::cout << "Arg 3 must be between 1 and 9" << std::endl;
            exit(1);
            // ERROR
        }
    }
    //init semophores
    sem_init(&rmutex, 0, 1);
    sem_init(&wmutex, 0, 1);
    sem_init(&readTry, 0, 1);
    sem_init(&resource, 0, 1);

    // TODO create arrays of reader and writer threads.
    // Create reader threads
    pthread_t* readers = new pthread_t[numReaders];
    pthread_t* writers = new pthread_t[numWriters];
    for(int i = 0; i < numReaders; i++){
        // Call readList on each thread
        // The second NULL here is the list of args
        pthread_create(&readers[i], NULL, readList, 0);

    }
    // Create Writer threads.
    int temp[] = {1,2,3,4,5,6,7,8,9};
    for(int i = 0; i < numWriters; i++){

        pthread_create(&readers[i], NULL, writeList, (void* ) &temp[i]) ;
        // Call writeList on each thread.
    }
}

