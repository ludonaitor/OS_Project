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

    Semaphore* sem = des -> semaphore;

    (sem -> count)++;

    if(sem->count <= 0) {

    SemDescriptorPtr* desptr = (SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors), (ListItem*) (sem->waiting_descriptors).first);

        PCB* pcb = desptr->descriptor->pcb;              //prendo il pcb del processo appena preso
        List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) desptr);
        List_detach(&waiting_list, (ListItem*) pcb);                               //lo rimuovo dalla lista di attesa...
        List_insert(&ready_list, (ListItem*) ready_list.last, (ListItem*) pcb);    //e lo inserisco in quello dei processi ready

        pcb->status = Ready;                                           //cambio lo status del processo

    }

    running->syscall_retvalue = 0;
    return;
}

