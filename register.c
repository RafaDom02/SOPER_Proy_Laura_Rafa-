/**
 * @file register.c
 *
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "register.h"
#include "miner.h"

int registrador(int fd, int rounds)
{
    int i;
    int j;
    ssize_t nbytes;
    char nombreArchivo[MAX_AUX];
    char minerToRegister[MAX_WORDS];
    int f;

    for (i = 0; i < rounds; i++)
    {
        while (TRUE)
        {
            nbytes = read(fd, minerToRegister, MAX_WORDS);
            if (nbytes == -1)
            {
                printf("Error en el read.\n");
                return EXIT_FAILURE;
            }
            if (strcmp(minerToRegister, "") != 0)
                break;
        }
        sprintf(nombreArchivo, "%d_bloque.txt", (int)getppid());

        f = open(nombreArchivo, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        if (f < 0)
        {
            printf("Error Number %d  %s\n", errno, nombreArchivo);
            perror("Program");
            close(fd);
            exit(EXIT_FAILURE);
        }
        dprintf(f, "%s", minerToRegister);
        dprintf(f, "\n\n");

        close(f);
    }
    close(fd);
    return EXIT_SUCCESS;
}
