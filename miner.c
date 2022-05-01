/**
 * @file miner.c
 *
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include "miner.h"
#include "pow.h"

/*****CONSTANTES DE ARGUMENTOS*****/
#define ARGS 3
#define MIN 0
#define MAX 1
#define NUM 2

/*****CONSTANTES PARA EL FOUND*****/
#define YES 1
#define NO 0

/*****CONSTANTES DE STRINGS*****/
#define STR_SIZE 8
#define RES_SIZE 1

/*****CONSTANTES PARA LA SALIDA DEL MONITOR*****/
// String de monitor a minero si ha aprobado el proceso
#define POSITIVE "P"
// String de monitor a minero si ha rechazado el proceso
#define NEGATIVE "N"

/*****CONSTANTES PARA LA SALIDA DEL MINERO*****/
#define FAILURE -1

/*****VARIABLES GLOBALES*****/
// Si se ha encontrado el número a buscar en la función hardwork()
int FOUND = NO;
Block *block;

void *hardwork(void *param)
{
    int i;
    int res;

    // Bucle desde el parametro min a max llamando a la funcion pow_hash()
    // donde comparamos todos los numeros del intervalo para encontrar el resultado deseado
    for (i = ((int *)param)[MIN]; i < ((int *)param)[MAX] && FOUND == NO; i++)
    {
        res = pow_hash(i);
        if(FOUND == YES) return NULL;
        if (res == ((int *)param)[NUM])
        {
            // printf("Se ha encontrado el numero %d\n", res);
            block->target = i;
            FOUND = YES;
            return NULL;
        }
    }
    return NULL;
}

void voting(int sig){
    
}

int minero(int rounds, int n_threads, int **fd, int fd_shm)
{
    int numperthr = POW_LIMIT / n_threads;
    int i;
    int j;
    int error;
    int param[n_threads][ARGS];
    char str[STR_SIZE + 1];
    char result[RES_SIZE + 1];
    size_t nbytes;
    pthread_t threads[n_threads];
    struct sigaction act_usr2;

    // Comprobación de errores
    if ( rounds <= 0 || n_threads <= 0)
        return EXIT_FAILURE;
    // Acceder a la memoria compartida
    block = (Block *)mmap(NULL, sizeof(block), PROT_READ, MAP_SHARED, fd_shm, 0);
    if (block == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    // Si el sistema esta lleno, salimos
    if (sem_trywait(&(block->sem_miners)) == -1)
    {
        return EXIT_SUCCESS;
    }
    //EMPIEZA AQUI
    
    act_usr2.sa_handler = voting;
    sigemptyset(&(act_usr2.sa_mask));
    act_usr2.sa_flags = 0;
    if (sigaction(SIGUSR2, &act_usr2, NULL) < 0)        //captura de señal SIGUSR2
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    //ACABA AQUI

    // Bucle donde nos aseguramos si se ha encontrado el número y hacemos que haga todas las rondas
    for (j = 0; j < rounds; j++)
    {
        sem_wait(&(block->sem_waiting));
        // Inicializamos por primera vez param asignando el target inicial
        for (i = 0; i < n_threads; i++)
            param[i][NUM] = block->target;

        // Por cada hilo le asignamos el intervalo de numeros que tienen que buscar y lo
        // creamos llamando a la funcion hardwork() con los parametros correspondientes
        for (i = 0; i < n_threads; i++)
        {
            param[i][MIN] = numperthr * i;
            if (i == n_threads - 1)
            {
                param[i][MAX] = POW_LIMIT;
                error = pthread_create(&threads[i], NULL, hardwork, param[i]);
                if (error != 0)
                {
                    fprintf(stderr, "pthread_create: %s\n", strerror(error));
                    sem_post(&(block->sem_miners));
                    return EXIT_FAILURE;
                }
            }
            else
            {
                param[i][MAX] = numperthr * (i + 1) - 1;
                error = pthread_create(&threads[i], NULL, hardwork, param[i]);
                if (error != 0)
                {
                    fprintf(stderr, "pthread_create: %s\n", strerror(error));
                    sem_post(&(block->sem_miners));
                    return EXIT_FAILURE;
                }
            }
        }
        // esperamos el retorno de todos los hilos
        for (i = 0; i < n_threads; i++)
        {
            error = pthread_join(threads[i], NULL);
            if (error != 0)
            {
                fprintf(stderr, "pthread_join: %s\n", strerror(error));
                sem_post(&(block->sem_miners));
                return EXIT_FAILURE;
            }
        }

        // si se encuentra y el numero de rondas ya ha terminado salimos de la función
        if (FOUND == YES)
        {

            // Pasamos como primer valor la posición del número obtenido
            sprintf(str, "%d", POSITION);
            nbytes = write(fd[0][1], str, STR_SIZE + 1);
            if (nbytes == -1)
            {
                printf("Error en el write.\n");
                sem_post(&(block->sem_miners));
                return EXIT_FAILURE;
            }
            // Pasamos como segundo valor la posición del número a buscar
            sprintf(str, "%d", param[0][NUM]);
            nbytes = write(fd[0][1], str, STR_SIZE + 1);
            if (nbytes == -1)
            {
                printf("Error en el write.\n");
                sem_post(&(block->sem_miners));
                return EXIT_FAILURE;
            }

            // Aqui minero recibe el mensaje de monitor de si su solucion ha sido rechazada o no
            nbytes = read(fd[1][0], result, RES_SIZE + 1);
            if (nbytes == -1)
            {
                printf("Error en el read.\n");
                sem_post(&(block->sem_miners));
                return EXIT_FAILURE;
            }
            if (strcmp(result, NEGATIVE) == 0)
            {
                printf("The solution has been invalidated\n");

                sprintf(str, "%d", FAILURE);
                nbytes = write(fd[0][1], str, STR_SIZE + 1);
                if (nbytes == -1)
                {
                    printf("Error en el write.\n");
                }
                sem_post(&(block->sem_miners));
                return EXIT_FAILURE;
            }

            FOUND = NO;
        }
    }

    sem_post(&(block->sem_miners));
    return EXIT_SUCCESS;
}