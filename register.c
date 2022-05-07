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

#define SIZE_WORD 32

int registrador(int fd, int rounds)
{
    int i;
    int j;
    ssize_t nbytes;
    char nombreArchivo[SIZE_WORD]; 
    char test[1];
    int f;

    for (i = 0; i < rounds; i++)
    {
        //Comprobamos que el padre consigue el bloque
        printf("TETDETYYWGFYWEGYEVW\n");
        do
        {
            nbytes = read(fd, test, 2);
            if (nbytes == -1)
            {
                printf("Error en el read.\n");
                return EXIT_FAILURE;
            }
            printf("777777777 %d ||| %d\n", getppid());
            i++;
        } while (shminfo->newblock.pidwinner != getppid());
        sprintf(nombreArchivo, "%d", (int)getppid());
        //Aqui ya sabemos que el padre es el ganador
        f = open(nombreArchivo, O_WRONLY | O_CREAT);
        if(f == -1){
            printf("Error Number % d\n", errno);
            perror("Program");
            close(fd);
            exit(EXIT_FAILURE);
        }
        printf("---------------------->%s\n", test);

        /*dprintf(f, "Id:             %d\n", shminfo->newblock.id);
        dprintf(f, "Winner:         %d\n", shminfo->newblock.pidwinner);
        dprintf(f, "Target:         %d\n", shminfo->newblock.target);
        if((shminfo->newblock.tvotes / 2) < shminfo->newblock.pvotes){
            dprintf(f, "Solution:       %d (validated) \n", shminfo->newblock.target);
        } else {
            dprintf(f, "Solution:       %d (rejected)\n", shminfo->newblock.target);
        }
        dprintf(f, "Votes:          %d/%d\n", shminfo->newblock.pvotes, shminfo->newblock.tvotes);
        dprintf(f, "Wallets:        ");
        for(j = 0; j < MAX_MINERS; j++){
            dprintf(f, "%d:%d ", shminfo->newblock.wallets[j]->pid, shminfo->newblock.wallets[j]->coins);
        }*/

        
        for(j = 0; j < MAX_MINERS; j++){
            if(shminfo->newblock.wallets[j] != NULL){
                printf("%d:%d ", shminfo->newblock.wallets[j]->pid, shminfo->newblock.wallets[j]->coins);
            }
        }
        
        close(f);
    }

    
    close(fd);
    return EXIT_SUCCESS;
}
