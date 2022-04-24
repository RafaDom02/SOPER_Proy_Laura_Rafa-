/**
 * @file main_miner.c
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

    // Alocamos memoria para las tuberías
    fd[0] = (int *)malloc(2 * sizeof(int));
    fd[1] = (int *)malloc(2 * sizeof(int));

    if (pipe(fd[0]) == -1)                                  //Tuberia direccion minero-registrador
    {
        printf("Error en la creación de la primera tubería.\n");
        return EXIT_FAILURE;
    }
    if (pipe(fd[1]) == -1)                                  //Tuberia direccion registrador-minero
    {
        printf("Error en la creación de la segunda tubería.\n");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid > 0)                                            // minero
    {                    
        close(fd[0][0]);                                    // Cerramos de la tuberia 1 la lectura.
        close(fd[1][1]);                                    // Cerramos de la tuberia 2 la escritura.
        if (!miner(0, atoi(argv[1]), atoi(argv[2]), fd));
            return EXIT_FAILURE;
    }
    else if (pid == 0)                                      // registrador
    {
        close(fd[0][1]);                                    // Cerramos de la tuberia 1 la escritura.
        close(fd[1][0]);                                    // Cerramos de la tuberia 2 la lectura.
        if (!registrador(fd));
            return EXIT_FAILURE;
    }
    else
    { // minero
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}