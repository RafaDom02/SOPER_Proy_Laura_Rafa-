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
#include "register.h"
#include "miner.h"


int main(int argc, char *argv[])
{
    int pid;
    int *fd[2];
    if (argc < 3)
    {
        printf("./miner <ROUNDS> <N_THREADS>");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid > 0)                                            // Comprobador
    {  
        if (!comprobador(atoi(argv[1]), atoi(argv[2]), fd));
            return EXIT_FAILURE;
    }
    else if (pid == 0)                                      // Monitor
    {
        if (!monitor(fd));
            return EXIT_FAILURE;
    }
    else
    { // minero
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}