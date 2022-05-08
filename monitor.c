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
#include "register.h"
#include "miner.h"

int monitor()
{
    SHM_mtc shmmtc;
    int fd_shm;

    shmmtc = (SHM_mtc *)mmap(NULL, sizeof(SHM_mtc), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0); // Obtiene el objeto SHM_mtc de la memoria compartida
    close(fd_shm);

    while (TRUE)
    {
        if(shmmtc.finalizando == 1){
            break;
        }
        sem_wait(&(shmmtc->sem_fill)); // Iniciamos parte Consumidor de Productor/Consumidor
        sem_wait(&(shmmtc->mutex));

        printf("Id:             %d\n", shmmtc->shminfo->newblock.id);
        printf("Winner:         %d\n", shmmtc->shminfo->newblock.pidwinner);
        printf("Target:         %ld\n", shmmtc->shminfo->newblock.target);
        if ((shminfo->newblock.tvotes / 2) < shmmtc->shminfo->newblock.pvotes)
        {
            printf("Solution:       %ld (validated) \n", shmmtc->shminfo->newblock.solution);
        }
        else
        {
            printf("Solution:       %ld (rejected)\n", shmmtc->shminfo->newblock.solution);
        }
        printf("Votes:          %d/%d\n", shmmtc->shminfo->newblock.pvotes, shmmtc->shminfo->newblock.tvotes);
        printf("Wallets:        ");
        for (i = 0; i < MAX_MINERS; i++)
        {
            if (shminfo->newblock.wallets[j] != NULL)
            {
                printf("%d:%d ", shmmtc->shminfo->newblock.wallets[j]->pid, shmmtc->shminfo->newblock.wallets[j]->coins);
            }
        }

        sem_post(&(shmmtc->mutex));
        sem_post(&(shmmtc->sem_empty)); // Terminamos parte Consumidor de Productor/Consumidor

    } // Cierra el fichero de la memoria compartida

    munmap(shmmtc, sizeof(shmmtc)); // Borramos el area mapeada de memoria compartida
    return EXIT_SUCCESS;
}