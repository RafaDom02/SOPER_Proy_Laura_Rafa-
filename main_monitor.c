/**
 * @file main_miner.c
 *
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// BIBLIOTECAS /////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> //Biblioteca para mmap()
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include "comprobador.h"
#include "monitor.h"

#define MAX_BUFFER 5

void shm_mtc_init(SHM_mtc *shmmtc)
{
    int i;
    if (sem_init(&(shmmtc->mutex), 1, 1) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_wait(&shmmtc->mutex);
    if (sem_init(&(shmmtc->sem_fill), 1, 0) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(shmmtc->sem_empty), 1, MAX_BUFFER) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    shmmtc->correcto = 0;
    shmmtc->finalizando = 0;
    shmmtc->shminfo = NULL;
    sem_post(&shmmtc->mutex);
}

int main(int argc, char *argv[])
{
    int pid;
    int fd_shm;
    SHM_mtc *shmmtc;

    pid = fork();
    // Creamos la memoria compartida
    fd_shm = shm_open(SHM_MTC_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd_shm == -1)
    {
        if (errno == EEXIST)
        {
            if ((fd_shm = shm_open(SHM_MTC_NAME, O_RDWR, 0)) == -1)
            {
                perror("Error opening the shared memory segment");
                exit(EXIT_FAILURE);
            }
            else
            {
                if (ftruncate(fd_shm, sizeof(shmmtc)) == -1)
                    return EXIT_FAILURE;
                // Obtenemos el bloque de la memoria compartida
                if ((shmmtc = (SHM_mtc *)mmap(NULL, sizeof(SHM_mtc), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0)) == MAP_FAILED)
                {
                    perror("mmap");
                    close(fd_shm);
                }
                shm_mtc_init(shmmtc);
                munmap(shmmtc, sizeof(shmmtc));
                if (!comprobador(fd_shm))
                    return EXIT_FAILURE;
                shm_unlink(SHM_MTC_NAME);
            }
        }
    }
    else
    {
        if (!monitor(fd_shm))
            return EXIT_FAILURE;
        wait(NULL);
    }

    return EXIT_SUCCESS;
}