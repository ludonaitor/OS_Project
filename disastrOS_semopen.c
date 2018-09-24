#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#include "disastrOS_globals.h"
#include "disastrOS_constants.h"

void internal_semOpen(){

    int id = running->syscall_args[0];    		//Primo Argomento sem_open
    int value = running->syscall_args[1]; 		//Secondo Argomento sem_open


    Semaphore* sem = SemaphoreList_byId(&semaphores_list, id);  	//Dalla lista globale dei semafori cerco tramite ID il semaforo

    if(!sem){
        sem = Semaphore_alloc(id, value);   			//Se non precedentemente allocato, lo aggiungo
        if(!sem){
            running -> syscall_retvalue = DSOS_SEMNOTALLOC;
            return;
        }

        List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem);   //Aggiungo il semaforo a semaphores_list
    }

    if (value < 0){   			//Controllo sul semaforo per validità valore
        running->syscall_retvalue = DSOS_SEMWRONGVALUE;
        return;
    }




	int fd = running->last_sem_fd;   //prossimo fd valido

	SemDescriptor* sem_desc = SemDescriptor_alloc(fd, sem, running);  //Alloco il SemDescriptor che si rifersce al semaforo
	if(!sem_desc){
        running->syscall_retvalue = DSOS_SEMNOTALLOC;
		return;
	}

	List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) sem_desc);
    //Aggiungo il descrittore allocato alla lista dei descrittori Sem_descriptors

	running->last_sem_fd++;  //Incremento il valore di last_sem_fd per mantenere la numerazione crescente.





	SemDescriptorPtr* sem_descptr = SemDescriptorPtr_alloc(sem_desc);  //Alloco il SemDescriptorPtr per le risorse
	if(!sem_descptr){
		running -> syscall_retvalue = DSOS_SEMNOTDESPTR;
		return;
    }

	sem_desc->ptr = sem_descptr;  //Collego il ptr di sem_desc con corrispettivo sem_desptr appena allocato


	List_insert(&sem->descriptors, sem->descriptors.last,(ListItem*) sem_descptr);  //Aggiungo alla ListHead Descriptors di sem il descrittore ptr.



	running->syscall_retvalue = sem_desc->fd;   //Retarno l' fd del descrittore sem_desc per identificare questo semaforo.
	return;

}
