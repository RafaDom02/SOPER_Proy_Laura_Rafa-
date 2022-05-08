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
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include "comprobador.h"

void shm_mtc_init(SHM_mtc *shmmtc)
{
    int i;
    if (sem_init(&(shmmtc->mutex), 1, 1) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(shmmtc->sem_fill), 1, 0) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(shmmtc->sem_empty), 1, MAX_MINERS) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int pid;
    int fd_shm;
    SHM_mtc shmmtc;

    if (argc != 1)
    {
        printf("./monitor");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid > 0) // comprobador
    {
        // Creamos la memoria compartida
        fd_shm = shm_open(SHM_MTC_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd_shm == -1)
        {
            if (errno == EEXIST)
            {
                if ((fd_shm = open(SHM_NAME, O_RDWR, 0)) == -1)
                {
                    perror("Error opening the shared memory segment");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    // Obtenemos el bloque de la memoria compartida
                    if ((shmmtc = (SHM_mtc *)mmap(NULL, sizeof(SHM_mtc), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
                    {
                        perror("mmap");
                        close(fd);
                    }
                    shm_mtc_init(shmmtc);
                    if (!comprobador(fd_shm))
                        return EXIT_FAILURE;
                }
            }
        }
        else
        {
            return EXIT_FAILURE;
        }
    }
    else if (pid == 0) // monitor
    {
        if (!monitor())
            return EXIT_FAILURE;
    }
    else
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}