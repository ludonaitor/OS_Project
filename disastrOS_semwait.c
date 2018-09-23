#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#include "linked_list.h" //inclusione funzioni Insert e Detach
#include "disastroOS_globals.h" //inclusione variabili globali
#include "disastrOS_constants" //inclusione costanti di messaggi di errore


void internal_semWait(){
  int fd = running->syscall_args[0];

  SemDescriptor* des = SemDescriptorList_byFd(&running-> sem_descriptors, fd);
  //Controllo errore descrittore semaforo non trovato
  if(!des){
    running->syscall_retvalue = DSOS_SEMNOTFD;
    return;
  }

  SemDescriptorPtr* descptr = des->ptr;
  //Controlla che il semaforo preso dal SemDescriptorPtr esista
  if(!descptr){
    running -> syscall_retvalue = DSOS_SEMNOTDESPTR;
    return;
  }

  //Controllo errore dell'esistenza del semafono preso dal SemDescriptor
  Semaphore* sem = des -> semaphore;
  if(!sem){
    running->syscall_retvalue = DSOS_SEMNOTSEM;
    return;
  }

  PCB* p;
  //Decremento il semaforo in quanto chiamata wait
  sem->count = (sem->count-1);
  if(sem->count < 0){
    List_detach(&sem->descriptors, (ListItem*)descptr); //rimuovo il descptr dalla lista descrittori
    List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem*)descptr); //inserisco lo stesso descptr nella waiting list dei descriptors
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);//il processo corrente è inserito nella waiting list all'ultimo posto
    running->status = Waiting;
    p = (PCB*)List_detach(&ready_list, (ListItem*)ready_list.first); //Assegno il primo PCB della ready list
    running = (PCB*)p; //p passa da waiting a running
  }

  running->syscall_retvalue = 0;
  return;
}
