#include <fstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <string.h>
#include <semaphore.h>
#include <time.h>

/***********************************************************************************************************/
/***********************************************************************************************************/

int readcount = 0, writecount = 0, numWrites = 0;                               //(initial value = 0)
pthread_cond_t condition;
sem_t rmutex, wmutex, readTry, resource; //(initial value = 1)                  //In the psuedocode P means wait and V means signal

/***********************************************************************************************************/
/***********************************************************************************************************/

struct Node{
    int data;
    Node* next;
};

Node* head=NULL, *tail=NULL;

/***********************************************************************************************************/
/***********************************************************************************************************/

void doNanoSleep(){
    nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
}


/***********************************************************************************************************/
/***********************************************************************************************************/
void* writeList(void* id1){
    int id = *(int*) id1;

    sem_wait(&wmutex); // Says we are writing
    writecount++;
    if (writecount == 1) // Checks if you're first writer
        sem_wait(&readTry); // Lock out the readers if we want to write
    sem_post(&wmutex); //release entry section
                            //<CRITICAL Section>
                        //writing is performed
    // TODO implement writing here in a way such that it doesnt cause problems
    for(int i = 0; i < numWrites; i++){
        sem_wait(&resource); //reserve the resource for yourself - prevents other writers from simultaneously editing the shared resource
        Node* temp = new Node;
        temp-> next = NULL;
        temp-> data = (rand() % 100) * 10 + id;
        if(tail == NULL){
            head = temp;
            tail = temp;
        }
        else{
            tail->next = temp;
            tail = temp;
        }
        sem_post(&resource); //release file
        doNanoSleep();
    }



    sem_wait(&wmutex); //reserve exit section
    writecount--; //indicate you're leaving
    if (writecount == 0) //checks if you're the last writer
        sem_post(&readTry); //if you're last writer, you must unlock the readers. Allows them to try enter CS for reading
    sem_post(&wmutex); //release exit section
    return NULL;
}

/***********************************************************************************************************/
/***********************************************************************************************************/

void readerHelper(FILE * fp, int reader_number, int iteration){

//<ENTRY Section>
    sem_wait(&readTry);                                               //Indicate a reader is trying to enter
    sem_wait(&rmutex);                                                //lock entry section to avoid race condition with other readers
    readcount++;                                                                //report yourself as a reader
    if(readcount == 1){                                                         //checks if you are first reader
        sem_wait(&resource);                                          //if you are first reader, lock  the resource
    }

    sem_post(&rmutex);                                              //release entry section for other readers
    sem_post(&readTry);                                             //indicate you are done trying to access the resource

//<CRITICAL Section>
//reading is performed

    int counter = 0;
    Node * position = new Node;
    position = head;
    while(place != NULL){
        if((position->data) % 10 == reader_number){counter++;}
        position = position->next;
    }

    fprintf(fp, "Reader %d: Read %d: %d values ending in %d\n", reader_number, iteration + 1, counter, reader_number);

//TODO: add wait/sleep

//<EXIT Section>
    sem_wait(&rmutex);                                                //reserve exit section - avoids race condition with readers
    readcount--;                                                                //indicate you're leaving
    if(readcount == 0){                                                       //checks if you are last reader leaving
        sem_post(&resource);                                         //if last, you must release the locked resource
    }else if(readcount == 1){
        pthread_cond_signal(&condition);
    }
    sem_post(&rmutex);                                              //release exit section for other readers
}

/***********************************************************************************************************/
/***********************************************************************************************************/

void * reader(void * payload){

    /* void * input is a int [2] array where:
    *       int[0] => reader_number
    *       int[1] => number of iterations reader reads from array
    */
    int reader_number = ((int *)payload[0]);
    int limit = ((int *)payload)[1];

    FILE * fp;
    char file_name[13];
    sprintf(file_name, "reader_%d.txt", reader);

    fp = fopen(file_name, "w");

    for(int i = 0; i < limit; i++){
        reader(fp, reader_limit, i);
    }

    fclose(fp);
}

/***********************************************************************************************************/
/***********************************************************************************************************/

void * extraThreadCall(void * payload){

    pthread_mutex_lock(&rmutex);
    pthread_cond_wait(&condition, &rmutex);
    fprintf(stdout, "Almost Done!");
    pthread_mutex_unlock(&rmutex);
}

/***********************************************************************************************************/
/***********************************************************************************************************/

int main(int argc, char* argv[]){
    int numReaders=0, numWriters=0;
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

    // Create Writer threads.
    int temp[] = {1,2,3,4,5,6,7,8,9};
    for(int i = 0; i < numWriters; i++){
        pthread_create(&writers[i], NULL, writeList, (void* ) &temp[i]) ;
        // Call writeList on each thread.
    }

    for(int i = 0; i < numReaders; i++){
        // Call readList on each thread
        // The second NULL here is the list of args
        pthread_create(&readers[i], NULL, readList, 0);

    }


    return 0;
}


/***********************************************************************************************************/
/***********************************************************************************************************/
/*DEAD CODE TELLS NO TALES*/
/*YEE BE WARNED*/

// int main(int argc, char** argv, char** envp){
//
//     pthread_t reader_thread;
//
//     int iterations;
//     int reader_limit;
//
//     int payload[2] = {iterations, 1};
//
//     for(int i = 0; i < reader_limit; i++){
//         payload[1]++;
//         pthread_create(&reader_thread, NULL, reader, (void *) payload);
//     }
//
//     return 0;
// }
