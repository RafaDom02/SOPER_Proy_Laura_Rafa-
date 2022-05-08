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
        nbytes = read(fd, minerToRegister, MAX_WORDS + 1);
        if (nbytes == -1)
        {
            printf("Error en el read.\n");
            return EXIT_FAILURE;
        }
        printf("me ha llegado lo de lectura y he leido: %ld\n", nbytes);
        sprintf(nombreArchivo, "%d_bloque", (int)getppid());

        f = open(nombreArchivo, O_WRONLY | O_TRUNC | O_CREAT | O_EXCL , S_IRUSR | S_IWUSR);
        if (f == -1)
        {
            printf("Error Number %d  %s\n", errno, nombreArchivo);
            perror("Program");
            close(fd);
            exit(EXIT_FAILURE);
        }
        ftruncate(f, MAX_WORDS);

        dprintf(f, "%s", minerToRegister);

        close(f);
    }

    close(fd);
    return EXIT_SUCCESS;
}
