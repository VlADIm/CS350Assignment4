#include <fstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <string.h>
#include <semaphore.h>
#include <time.h>

/***********************************************************************************************************/
/***********************************************************************************************************/

int readcount = 0, writecount = 0, numWrites = 0, numReaders = 0;
pthread_cond_t condition;
sem_t wmutex, readTry, resource; //In the psuedocode P means wait and V means signal
pthread_mutex_t rmutex, extraMutex;

/***********************************************************************************************************/
/***********************************************************************************************************/

struct Node{
    int data;
    Node* next;
};

Node* head=NULL, *tail=NULL;

/***********************************************************************************************************/
/***********************************************************************************************************/

// Sleeps half a second.
void doNanoSleep(){
    struct timespec temp {0, rand()};
    nanosleep(&temp, NULL);
    //nanosleep((const struct timespec[]){{0, 0}}, NULL);
}


/***********************************************************************************************************/
/***********************************************************************************************************/
void* writeList(void* info){
    // Get thread ID
    int id = *(int*) info;

    std::cout << "Writer " << id << " created" << std::endl;

    for(int i = 0; i < numWrites; i++){
        std::cout << "Writer " << id << " requesting wmutex" << std::endl;
        sem_wait(&wmutex); // Says we are writing
        writecount++;
        if (writecount == 1){ // Checks if you're first writer
            std::cout << "Writer " << id << " requesting readtry" << std::endl;
            sem_wait(&readTry); // Lock out the readers if we want to write
        }
        std::cout << "Writer " << id << " releasing wmutex" << std::endl;
        sem_post(&wmutex); // Release entry section

            Node* temp = new Node;
            temp-> next = NULL;
            temp-> data = (rand() % 100) * 10 + id;

            std::cout << "Writer " << id << " requesting resource" << std::endl;
            sem_wait(&resource); // Reserve the resource for yourself 

                                /*<CRITICAL Section>*/
            // If this is the first node, set head and tail.
            if(tail == NULL){
                head = temp;
                tail = temp;
            }
            else{
                tail->next = temp;
                tail = temp;
            }
            std::cout << "Writer " << id << " releasing resource" << std::endl;
            sem_post(&resource); // Let others write before we sleep.
            // Sleep so we can see the effects
        std::cout << "Writer " << id << " get wmutex" << std::endl;
        sem_wait(&wmutex); // Aquire writecount lock to leave
        writecount--; 
        if (writecount == 0){
            std::cout << "Writer " << id << " releasing readTry" << std::endl;
            sem_post(&readTry); // If you're last writer, you must unlock the readers.
        }
        std::cout << "Writer " << id << " release wmutex" << std::endl;
        sem_post(&wmutex); // Release writecount lock

        doNanoSleep();
    }
    return NULL;
}

/***********************************************************************************************************/
/***********************************************************************************************************/

void readerHelper(FILE * fp, int reader_number, int iteration){

//<ENTRY Section>
    std::cout << "Reader " << reader_number << " requesting readtry" << std::endl;
    sem_wait(&readTry);                                               //Indicate a reader is trying to enter
    std::cout << "Reader " << reader_number << " requesting rmutex" << std::endl;
    pthread_mutex_lock(&rmutex);                                                //lock entry section to avoid race condition with other readers
    readcount++;                                                                //report yourself as a reader
    if(readcount == 1){                                                         //checks if you are first reader
        std::cout << "Reader " << reader_number << " requesting readcount" << std::endl;
        sem_wait(&resource);                                          //if you are first reader, lock  the resource
    }

    std::cout << "Reader " << reader_number << " releasing rmutex" << std::endl;
    pthread_mutex_unlock(&rmutex);                                              //release entry section for other readers
    std::cout << "Reader " << reader_number << " releasing readTry" << std::endl;
    sem_post(&readTry);                                             //indicate you are done trying to access the resource

//<CRITICAL Section>
//reading is performed

    int counter = 0;
    Node * position = head;
    while(position != NULL){
        if((position->data) % 10 == reader_number){counter++;}
        position = position->next;
    }

    fprintf(fp, "Reader %d: Read %d: %d values ending in %d\n", reader_number, iteration + 1, counter, reader_number);

//TODO: add wait/sleep

//<EXIT Section>
    std::cout << "Reader " << reader_number << " requesting rmutex" << std::endl;
    pthread_mutex_lock(&rmutex);                                                //reserve exit section - avoids race condition with readers
    readcount--;                                                                //indicate you're leaving
    if(readcount == 0){                                                       //checks if you are last reader leaving
        std::cout << "Reader " << reader_number << " releasing resource" << std::endl;
        sem_post(&resource);                                         //if last, you must release the locked resource
    }
    std::cout << "Reader " << reader_number << " releasing rmutex" << std::endl;
    pthread_mutex_unlock(&rmutex);                                              //release exit section for other readers
}

/***********************************************************************************************************/
/***********************************************************************************************************/

void * readList(void * payload){
    std::cout << "Reader " << (((int *)payload)[1]) << " created" << std::endl;

    /* void * input is a int [2] array where:
    *       int[0] => reader_number
    *       int[1] => number of iterations reader reads from array
    */
    int iterations = (((int *)payload))[0];
    int reader_number = (((int *)payload)[1]);

    FILE * fp;
    char file_name[13];
    sprintf(file_name, "reader_%d.txt", reader_number);

    fp = fopen(file_name, "w");

    for(int i = 0; i < iterations; i++){
        readerHelper(fp, reader_number, i);
        doNanoSleep();
    }

    pthread_mutex_lock(&extraMutex);
    numReaders--;
    if(numReaders == 1){
        std::cout << "Condition" << std::endl;
        pthread_cond_signal(&condition);
    }
    pthread_mutex_unlock(&extraMutex);

    fclose(fp);
    return NULL;
}

/***********************************************************************************************************/
/***********************************************************************************************************/

void * extraThreadCall(void * payload){

    pthread_mutex_lock(&rmutex);
    pthread_cond_wait(&condition, &rmutex);
    fprintf(stdout, "Almost Done!");
    pthread_mutex_unlock(&rmutex);
    return NULL;
}

/***********************************************************************************************************/
/***********************************************************************************************************/

int main(int argc, char* argv[]){
    int numWriters=0;
    if(argc != 4){
        //NO ARGS
        std::cout << "Argc: " << argc << ". Argc should be 4" << std::endl;
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
        numWriters = temp;
        
    }
    //init semophores
    pthread_mutex_init(&rmutex,NULL);
    pthread_mutex_init(&extraMutex,NULL);
    sem_init(&wmutex, 0, 1);
    sem_init(&readTry, 0, 1);
    sem_init(&resource, 0, 1);

    // TODO create arrays of reader and writer threads.
    // Create reader threads
    pthread_t* readers = new pthread_t[numReaders];
    pthread_t* writers = new pthread_t[numWriters];

    int iterations = numWrites;
    int payloadHolder[9][2];

    std::cout << "Creating " << numReaders << " Readers." << std::endl;
    for(int i = 0; i < numReaders; i++){
        payloadHolder[i][0] = iterations;
        payloadHolder[i][1] = i+1;
        // Call readList on each thread
        // The second NULL here is the list of args
        pthread_create(&readers[i], NULL, readList, (void *) payloadHolder[i]);
        std::cout << "Reader num " << payloadHolder[i][1] << " initialized." << std::endl;
    }

    // Create Writer threads.
    int temp[] = {1,2,3,4,5,6,7,8,9};

    std::cout << "Creating " << numWriters << " Writers." << std::endl;
    for(int i = 0; i < numWriters; i++){
        pthread_create(&writers[i], NULL, writeList, (void* ) &temp[i]) ;
        // Call writeList on each thread.
    }
    pthread_t observer;
    pthread_create(&observer, NULL, extraThreadCall, NULL) ;


/***********************************************************************************************************/
/***********************************************************************************************************/
    // Joining threads
    for(int i = 0; i < numWriters; i++){
        pthread_join(writers[i], NULL);
    }

    for(int i = 0; i < numReaders; i++){
        pthread_join(readers[i], NULL);
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
