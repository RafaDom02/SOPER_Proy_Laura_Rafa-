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

/**
 * @brief Inicializamos los semaforos sem_empty, sem_fill y mutex
 * 
 * @param sem_empty Semaphore
 * @param sem_fill Semaphore
 * @param mutex Semaphore
 * @return EXIT_FAILURE or EXIT_SUCCCESS
 */
int crearSemaforos(sem_t *sem_empty, sem_t *sem_fill, sem_t *mutex)
{
    // Creacion del primer semaforo
    if (sem_init(sem_empty, 1, MAX_BUFFER) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Creacion del segundo semaforo
    if (sem_init(sem_fill, 1, 0) == -1)
    {
        sem_destroy(sem_empty);
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Creacion del mutex
    if (sem_init(mutex, 1, 1) == -1)
    {
        sem_destroy(sem_empty);
        sem_destroy(sem_fill);
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int pid;
    int fd_shm;

    if (argc != 1)
    {
        printf("./monitor");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid > 0) // comprobador
    {
        fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd_shm == -1)
        {
            if (errno == EEXIST)
            {
                if ((fd_shm = open(SHM_NAME, O_RDWR, 0)) == -1){
                    perror ("Error opening the shared memory segment");
                    exit(EXIT_FAILURE);
                }
                else{
                    return EXIT_FAILURE;
                }
            }
        }
        else
        {
            if (!comprobador(fd_shm))
                return EXIT_FAILURE;
        }
    }
    else if (pid == 0) // monitor
    {
        if(!monitor())
            return EXIT_FAILURE;
    }
    else{
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}