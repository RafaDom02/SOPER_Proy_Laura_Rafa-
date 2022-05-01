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
#include <stdio.h>  //Biblioteca base
#include <stdlib.h> //Biblioteca para funciones adicionales
#include <time.h>   //Biblioteca para nanosleep()
#include <unistd.h>
#include <sys/mman.h> //Biblioteca para mmap()
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "register.h"
#include "miner.h"

#define SHM_NAME "/shm_minero_registrador"

void block_init(Block **block)
{
    if (sem_init(&(*block)->sem_waiting, 1, 0) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(*block)->sem_miners, 1, MAX_MINERS) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    (*block)->id = 0;
    (*block)->target = 0;
}

int main(int argc, char *argv[])
{
    int pid;
    int *fd[2];
    int fd_shm;
    Block *block = NULL;
    sem_t sem_miners;

    if (argc < 3)
    {
        printf("./miner <ROUNDS> <N_THREADS>");
        return EXIT_FAILURE;
    }

    // Alocamos memoria para las tuberías
    fd[0] = (int *)malloc(2 * sizeof(int));
    fd[1] = (int *)malloc(2 * sizeof(int));

    if (pipe(fd[0]) == -1) // Tuberia direccion minero-registrador
    {
        printf("Error en la creación de la primera tubería.\n");
        return EXIT_FAILURE;
    }
    if (pipe(fd[1]) == -1) // Tuberia direccion registrador-minero
    {
        printf("Error en la creación de la segunda tubería.\n");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid > 0) // minero
    {
        close(fd[0][0]); // Cerramos de la tuberia 1 la lectura.
        close(fd[1][1]); // Cerramos de la tuberia 2 la escritura.
        fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd_shm == -1) // LOS DEMAS MINEROS
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
                    if (!miner(atoi(argv[1]), atoi(argv[2]), fd, fd_shm))
                        return EXIT_FAILURE;
                }
            }
        }
        else // PRIMER MINERO
        {
            if (ftruncate(fd_shm, sizeof(block)) == -1)
                return EXIT_FAILURE;
            block = (Block *)mmap(NULL, sizeof(block), PROT_READ, MAP_SHARED, fd_shm, 0);
            if (block == MAP_FAILED)
            {
                perror("mmap");
                exit(EXIT_FAILURE);
            }
            block_init(&block);
            munmap(block, sizeof(block));
            if (!miner(atoi(argv[1]), atoi(argv[2]), fd, fd_shm))
                return EXIT_FAILURE;
        }
    }
    else if (pid == 0) // registrador
    {
        close(fd[0][1]); // Cerramos de la tuberia 1 la escritura.
        close(fd[1][0]); // Cerramos de la tuberia 2 la lectura.
        if (!registrador(fd))
            return EXIT_FAILURE;
    }
    else
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}