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
// Posición del número encontrado en hardwork()
int POSITION;

void *hardwork(void *param)
{
    int i;
    int res;

    // Bucle desde el parametro min a max llamando a la funcion pow_hash()
    // donde comparamos todos los numeros del intervalo para encontrar el resultado deseado
    for (i = ((int *)param)[MIN]; i < ((int *)param)[MAX] && FOUND == NO; i++)
    {
        res = pow_hash(i);
        if (res == ((int *)param)[NUM])
        {
            // printf("Se ha encontrado el numero %d\n", res);
            POSITION = i;
            FOUND = YES;
        }
    }
    return NULL;
}



int minero(int target, int rounds, int n_threads, int **fd)
{
    //1: los hilos tienen que buscar la POW
    int numperthr = POW_LIMIT / n_threads;
    int i;
    int j;
    int error;
    int param[n_threads][ARGS];
    char str[STR_SIZE + 1];
    char result[RES_SIZE + 1];
    ssize_t nbytes;
    pthread_t threads[n_threads];

    // Comprobación de errores
    if (target < 0 || target > POW_LIMIT || rounds <= 0 || n_threads <= 0)
        return EXIT_FAILURE;

    // Bucle donde nos aseguramos si se ha encontrado el número y hacemos que haga todas las rondas
    for (j = 0; j < rounds; j++)
    {
        // Inicializamos por primera vez param asignando el target inicial
        if (j == 0)
            for (i = 0; i < n_threads; i++)
                param[i][NUM] = target;
        // Para las demas rondas ponemos el target en la posicion de la anterior ronda
        else
            for (i = 0; i < n_threads; i++)
                param[i][NUM] = POSITION;

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
                return EXIT_FAILURE;
            }
            // Pasamos como segundo valor la posición del número a buscar
            sprintf(str, "%d", param[0][NUM]);
            nbytes = write(fd[0][1], str, STR_SIZE + 1);
            if (nbytes == -1)
            {
                printf("Error en el write.\n");
                return EXIT_FAILURE;
            }

            // Aqui minero recibe el mensaje de monitor de si su solucion ha sido rechazada o no
            nbytes = read(fd[1][0], result, RES_SIZE + 1);
            if (nbytes == -1)
            {
                printf("Error en el read.\n");
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
                return EXIT_FAILURE;
            }
            

            FOUND = NO;
        }
    }

    return EXIT_SUCCESS;

    //2: Si un hilo la encuentra, manda a los demas
    //  a validar su solucion mediante una votacion



    //3: Validacion correcta = moneda y enviar el bloque
    //  al monitor (tiene que estar activo) mediante una 
    //  cola de mensajes

    
    return EXIT_SUCCESS;
}