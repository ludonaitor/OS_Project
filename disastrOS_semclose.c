#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){

  //Descrittore della sem_close
  int fd = running->syscall_args[0];

  //Variabile di controllo errore
  int controllo;

  //Ricerco il SemDescriptor tramite il suo fd
  SemDescriptor* sem_desc = SemDescriptorList_byFd(&running->sem_descriptors, fd);

  //Controllo errore sem_desc non trovato
  if(!sem_desc){
    running-> syscall_retvalue = DSOS_SEMNOTFD;
    return;
  }

  //Semaforo corrispondente al descrittore sem_desc
  Semaphore* sem = sem_desc->semaphore;

  //Controllo errore semaforo non trovato
  if(!sem){
    running->syscall_retvalue = DSOS_SEMNOTSEM;
    return;
  }

  //Descrittore SemDescriptor preso da sem_desc
  SemDescriptorPtr* sem_descptr = sem_desc->ptr;

  //Controllo errore sem_descptr
  if(!sem_descptr){
    running->syscall_retvalue = DSOS_SEMNOTDESPTR;
    return;
  }

  sem_desc = (SemDescriptor*) List_detach(&running->sem_descriptors, (ListItem*) sem_desc);

  //Rilascio delle risorse del SemDescriptor
  controllo = SemDescriptor_free(sem_desc);

  //Controllo errore rilascio risorse
  if(controllo){
    running->syscall_retvalue = DSOS_SEMNOTFREE;
    return;
  }

  //Eliminazione della lista dei descrittori del semaforo il riferimento a SemDescriptorPtr
  sem_descptr = (SemDescriptorPtr*) List_detach(&sem->descriptors, (ListItem*)sem_descptr);

  //Se non ci sono piu descrittori in running e in attesa rilasico le risorse
  if(sem->descriptors.size == 0 && sem->waiting_descriptors.size == 0){
    sem = (Semaphore*)List_detach(&semaphores_list, (ListItem*)sem);
    controllo = Semaphore_free(sem);
    if(controllo){
      running->syscall_retvalue = DSOS_SEMNOTFREE;
      return;
    }
  }

  //Rilascio delle risorse del SemDescriptorPtr
  controllo = SemDescriptorPtr_free(sem_descptr);
  if(controllo){
    running->syscall_retvalue = DSOS_SEMNOTFREE;
    return;
  }

  //Return 0 in caso di successo
  running->syscall_retvalue = 0;
  return;
}
