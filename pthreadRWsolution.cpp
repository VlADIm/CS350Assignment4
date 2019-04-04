#include <ofstream>
#include <ifstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <string.h>

int readcount = 0, writecount = 0;                   //(initial value = 0)
// semaphore rmutex, wmutex, readTry, resource; //(initial value = 1)
pthread_condition_t condition;
pthread_mutex_lock_t rmutex, wmutex, readTry, resource, extraThread;

//READER
//reader() {
//<ENTRY Section>
//  readTry.P();                 //Indicate a reader is trying to enter
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
//
//        //WRITER
//        writer() {
// <ENTRY Section>
// wmutex.P();                  //reserve entry section for writers - avoids race conditions
// writecount++;                //report yourself as a writer entering
//              if (writecount == 1)         //checks if you're first writer
//                  readTry.P();               //if you're first, then you must lock the readers out. Prevent them from trying to enter CS
//                    wmutex.V();                  //release entry section
//
//                    <CRITICAL Section>
//                      resource.P();                //reserve the resource for yourself - prevents other writers from simultaneously editing the shared resource
//                        //writing is performed
//                          resource.V();                //release file
//
//                          <EXIT Section>
//                            wmutex.P();                  //reserve exit section
//                              writecount--;                //indicate you're leaving
//                                if (writecount == 0)         //checks if you're the last writer
//                                    readTry.V();               //if you're last writer, you must unlock the readers. Allows them to try enter CS for reading
//                                    wmutex.V();                  //release exit section
// }

/***********************************************************************************************************/
/***********************************************************************************************************/
void readerHelper(FILE * fp, int reader_number, int iteration){



//<ENTRY Section>
    pthread_mutex_lock(&readTry);                                               //Indicate a reader is trying to enter
    pthread_mutex_lock(&rmutex);                                                //lock entry section to avoid race condition with other readers
    readcount++;                                                                //report yourself as a reader
    if(readcount == 1){                                                         //checks if you are first reader
        pthread_mutex_lock(&resource);                                          //if you are first reader, lock  the resource
    }

    pthread_mutex_unlock(&rmutex);                                              //release entry section for other readers
    pthread_mutex_unlock(&readTry);                                             //indicate you are done trying to access the resource

//<CRITICAL Section>
//reading is performed
    // Linkedlist * containter = LINKEDLISTHEAD
    // counter = 0;
    // for(i = 0; i < limit && (*containter.next) != NULL; i++){
    //     if((*containter).data % 10 == reader_number) counter++;
    //     containter = (*containter).next
    // }
    //
    // fprintf(fp, "Reader %d: Read %d: %d values ending in %d\n", reader_number, iteration + 1, counter, reader_number);


//<EXIT Section>
    pthread_mutex_lock(&rmutex);                                                //reserve exit section - avoids race condition with readers
    readcount--;                                                                //indicate you're leaving
    if(readcount == 0){                                                       //checks if you are last reader leaving
        pthread_mutex_unlock(&resource);                                         //if last, you must release the locked resource
    }else if(readcount == 1){
        pthread_cond_signal(&condition);
    }
    pthread_unlock_mutex(&rmutex);                                              //release exit section for other readers
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

void * extraThreadCall(void * payload){

    pthread_mutex_lock(&rmutex);
    pthread_cond_wait(&condition, &rmutex);
    fprintf(stdout, "Almost Done!");
    pthread_mutex_unlock(&rmutex);
}

int main(int argc, char** argv, char** envp){

    pthread_t reader_thread;

    int iterations;
    int reader_limit;

    int payload[2] = {iterations, 1};

    for(int i = 0; i < reader_limit; i++){
        payload[1]++;
        pthread_create(&reader_thread, NULL, reader, (void *) payload);
    }

    return 0;
}
/***********************************************************************************************************/
/***********************************************************************************************************/
