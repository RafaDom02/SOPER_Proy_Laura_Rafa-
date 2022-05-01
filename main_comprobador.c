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
#include <sys/mman.h> //Biblioteca para mmap()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "comprobador.h"

#define SHM_NAME "/shm_comprobador_monitor"

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
                    if(!)
                        return EXIT_FAILURE;
                }
            }
        }
        else
        {

            if (!comprobador(atoi(argv[1]), atoi(argv[2])))
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