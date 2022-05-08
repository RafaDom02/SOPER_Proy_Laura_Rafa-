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
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <mqueue.h>
#include "register.h"
#include "miner.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// MINERO/REGISTRADOR ////////////////////////////////////////////////////////////////////

void shm_info_init(SHM_info *shminfo)
{
    int i;
    if (sem_init(&(shminfo->mutex), 1, 1) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_wait(&shminfo->mutex);
    if (sem_init(&(shminfo->sem_waiting), 1, 0) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(shminfo->sem_miners), 1, MAX_MINERS) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    shminfo->minersvoting = 0;
    shminfo->newblock.id = 0;
    shminfo->newblock.target = 0;
    shminfo->newblock.solution = -1;
    shminfo->prevblock.pidwinner = getpid();
    for (i = 0; i < MAX_MINERS; i++)
    {
        shminfo->newblock.wallets[i] = NULL;
        shminfo->pids_esperando[i] = 0;
        shminfo->pids_minando[i] = 0;
        shminfo->votes[i] = 0;
    }
    sem_post(&shminfo->mutex);
}

int main(int argc, char *argv[])
{
    int pid;
    int fd[2];
    int fd_shm;
    SHM_info *shminfo = NULL;
    int group;
    int ppid;

    if (argc < 3)
    {
        printf("./miner <ROUNDS> <N_THREADS>\n");
        return EXIT_FAILURE;
    }

    if (pipe(fd) == -1) // Tuberia direccion minero-registrador
    {
        printf("Error en la creación de la primera tubería.\n");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid > 0) // minero
    {
        close(fd[0]);                                                              // Cerramos de la tuberia la lectura.
        fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR); // Creamos la memoria compartida
        if (fd_shm == -1)                                                          // LOS DEMAS MINEROS
        {
            if (errno == EEXIST)
            {
                if ((fd_shm = shm_open(SHM_NAME, O_RDWR, 0)) == -1) // Como existe, solo tenemos que abrir
                {
                    perror("Error opening the shared memory segment");
                    printf("Error con el pid %d\n", getpid());
                    close(fd[1]);
                    wait(NULL);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    if (!miner(atoi(argv[1]), atoi(argv[2]), fd[1], fd_shm)){
                        close(fd[1]);
                        return EXIT_FAILURE;
                    }
                        
                    wait(NULL);
                }
            }
        }
        else // PRIMER MINERO
        {
            if (ftruncate(fd_shm, sizeof(shminfo)) == -1)
                return EXIT_FAILURE;
            shminfo = (SHM_info *)mmap(NULL, sizeof(shminfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
            if (shminfo == MAP_FAILED)
            {
                perror("mmap");
                close(fd[1]);
                wait(NULL);
                exit(EXIT_FAILURE);
            }
            shm_info_init(shminfo);
            munmap(shminfo, sizeof(shminfo));
            if (!miner(atoi(argv[1]), atoi(argv[2]), fd[1], fd_shm)){
                close(fd[1]);
                return EXIT_FAILURE;
            }
                
            wait(NULL);
        }
    }
    else if (pid == 0) // registrador
    {
        close(fd[1]); // Cerramos de la tuberia la escritura.
        if (!registrador(fd[0], atoi(argv[1]))){
            close(fd[0]);
            return EXIT_FAILURE;
        }
            
    }
    else
    {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        exit(EXIT_FAILURE);
    }
    close(fd[0]);
    close(fd[1]);
    return EXIT_SUCCESS;
}