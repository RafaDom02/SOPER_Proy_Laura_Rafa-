/**
 * @file monitor.c
 *
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include "comprobador.h"
#include "monitor.h"


int monitor(int fd_shm)
{
    SHM_mtc *shmmtc;
    int i = 0;

    // Obtiene el objeto SHM_mtc de la memoria compartida
    shmmtc = (SHM_mtc *)mmap(NULL, sizeof(SHM_mtc), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0); 
    close(fd_shm);
    sleep(1);
    while (TRUE)
    {
        sem_wait(&(shmmtc->sem_fill)); // Iniciamos parte Consumidor de Productor/Consumidor
        sem_wait(&(shmmtc->mutex));
        //Si obtenemos un bloque donde finalizando este a 1, terminamos la ejecucion
        if(shmmtc->finalizando == 1){
            break;
        }
        //Printeamos los valores del bloque obtenido por memoria compartida
        printf("Id:             %d\n", shmmtc->shminfo->id);
        printf("Winner:         %d\n", shmmtc->shminfo->pidwinner);
        printf("Target:         %ld\n", shmmtc->shminfo->target);
        if ((shmmtc->shminfo->tvotes / 2) < shmmtc->shminfo->pvotes)
        {
            printf("Solution:       %ld (validated) \n", shmmtc->shminfo->solution);
        }
        else
        {
            printf("Solution:       %ld (rejected)\n", shmmtc->shminfo->solution);
        }
        printf("Votes:          %d/%d\n", shmmtc->shminfo->pvotes, shmmtc->shminfo->tvotes);
        printf("Wallets:        ");
        for (i = 0; i < MAX_MINERS; i++)
        {
            if (shmmtc->shminfo->wallets[i] != NULL)
            {
                printf("%d:%d ", shmmtc->shminfo->wallets[i]->pid, shmmtc->shminfo->wallets[i]->coins);
            }
        }

        sem_post(&(shmmtc->mutex));
        sem_post(&(shmmtc->sem_empty)); // Terminamos parte Consumidor de Productor/Consumidor

    } // Cierra el fichero de la memoria compartida

    munmap(shmmtc, sizeof(shmmtc)); // Borramos el area mapeada de memoria compartida
    return EXIT_SUCCESS;
}