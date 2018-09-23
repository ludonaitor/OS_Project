#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
    int fd = running -> syscall_args[0];
    SemDescriptor* des = SemDescriptorList_byFd(&running->sem_descriptors,fd);
    if(!des) {
        running -> syscall_retvalue = DSOS_SEMNOTFD;
    }

    Semaphore* sem = des -> semaphore;
    if(!sem) {
        running -> syscall_retvalue = DSOS_SEMNOTSEM;
    }

    (sem -> count)++;

    if(sem->count <= 0) {

    SemDescriptorPtr* desptr = (SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors), (ListItem*) (sem->waiting_descriptors).first);

        PCB* pcb = desptr->descriptor->pcb;              //assegno il pcb attualmente in running
        List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desptr);
        List_detach(&waiting_list, (ListItem*) pcb);                               //rimuovo dalla lista di attesa
        List_insert(&ready_list, (ListItem*) ready_list.last, (ListItem*) pcb);    //lo inserisco in quello dei processi ready

        pcb->status = Ready;                                           //assegno alo stato del pcb il nuovo stato attuale

    }

    running->syscall_retvalue = 0;
    return;
}

