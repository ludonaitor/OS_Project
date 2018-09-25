#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>

#include "disastrOS.h"
#include "disastrOS_globals.h"


#define BUFFER_SIZE 5
#define MAX_TRANSACTION 100
#define CICLES 5

//Inseriamo variabili per il buffer circolare da inserire nel test produttore/consumatore
int buf[BUFFER_SIZE];
int read_index, write_index;
int sum;


static inline int performRandomTransaction() {

    int amount = rand() % (2 * MAX_TRANSACTION);
    if (amount++ >= MAX_TRANSACTION) {
        return MAX_TRANSACTION - amount;
    } else {
        return amount;
    }
}

void producer(int sem_empty, int sem_filled, int sem_prod){

  for(int i = 0; i < CICLES; i++){

    int randnum = performRandomTransaction();
    disastrOS_semwait(sem_empty);

    printf("------CICLO %d PRODUZIONE INIZIATO-------\n\n", i);

    disastrOS_semwait(sem_prod);

    buf[write_index] = randnum;
    write_index = (write_index + 1) % BUFFER_SIZE;

    disastrOS_sempost(sem_prod);

    printf("------PRODOTTO------\n");
    printf("Il processo produttore %d",disastrOS_getpid());
    printf(" ha prodotto la risorsa %d\n\n",randnum);


    disastrOS_sempost(sem_filled);

    printf("---------CICLO %d PRODUZIONE FINITO-------- \n\n", i);
  }

}

void consumer(int sem_empty, int sem_filled, int sem_cons){

  for(int i = 0; i < CICLES; i++){

    disastrOS_semwait(sem_filled);

    printf("-------CICLO %d CONSUMATORE INIZIATO--------\n\n", i);

    disastrOS_semwait(sem_cons);

    int last = buf[read_index];
    sum+=last;
    buf[read_index] = 0;
    read_index = (read_index + 1) % BUFFER_SIZE;

    disastrOS_sempost(sem_cons);

    printf("-------CONSUMATO--------\n");
    printf("La somma è: %d\n\n",sum);

    disastrOS_sempost(sem_empty);

    printf("-----------CICLO %d CONSUMATORE TERMINATO---------\n\n", i);

  }

}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}



void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);


read_index = 0;
write_index = 0;

int sem_filled = disastrOS_semopen(1,0);

int sem_empty = disastrOS_semopen(2, BUFFER_SIZE);

int sem_cons = disastrOS_semopen(3, 1);

int sem_prod = disastrOS_semopen(4, 1);

disastrOS_printStatus();

if(disastrOS_getpid() > 1){
  if(disastrOS_getpid() % 2 == 0){

    printf("-------PROCESSO PRODUTTORE %d AVVIATO-------\n\n", disastrOS_getpid());
    disastrOS_sleep(15);
    producer(sem_empty, sem_filled, sem_prod);
  }
  else{
    printf("---------PROCESSO CONSUMATORE %d AVVIATO-------\n\n", disastrOS_getpid());
    consumer(sem_empty, sem_filled, sem_cons);
  }
}

  printf("PID: %d, terminating\n", disastrOS_getpid());

/*
  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  */
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);


  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<5; ++i) {
    //int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    //int fd=disastrOS_openResource(i,type,mode);
    //printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  //disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
    //disastrOS_printStatus();
    //printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   //pid, retval, alive_children);
    --alive_children;
  }
  disastrOS_printStatus();
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
