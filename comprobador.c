/**
 * @brief Comprobador:
 *
 * @author Rafael Domínguez Saez rafael.dominguez@estudiante.uam.es
 * @author Laura María García Suárez lauramaria.garcias@estudiante.uam.es
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// BIBLIOTECAS /////////////////////////////////////////////////////////////////////
#include <stdio.h>     //Biblioteca base
#include <stdlib.h>    //Biblioteca para funciones adicionales
#include <time.h>      //Biblioteca para nanosleep()
#include <sys/mman.h>  //Biblioteca para mmap()
#include <unistd.h>    //Biblioteca para ftruncate()
#include <sys/stat.h>  //Biblioteca para sem_open()
#include <fcntl.h>     //Biblioteca para sem_open()
#include <sys/types.h> //Biblioteca para sem_wait()
#include <signal.h>
#include <mqueue.h>

#include "mintocom.h"
#include "comprobador.h" //Biblioteca de comprobador
#include "pow.h"         //Biblioteca para hash_pow()

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// VARIABLES GLOBALES /////////////////////////////////////////////////////////////////////

SHM_mtc *shmmtc;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// HANDLERS /////////////////////////////////////////////////////////////////////

/**
 * @brief Cuando llega la señal SIGINT
 *
 * @param sig Valor de la señal SIGINT
 */
void handler(int sig)
{
    shmmtc->finalizando = 1;
    exit(EXIT_SUCCESS);
}

/**
 * @brief Aplicacion de las señales al proceso
 *
 * @param act_int puntero a estructura que define el comportamiento cuando se reciba la señal SIGINT
 */
void auxHandlerSI(struct sigaction act_int)
{
    act_int.sa_handler = handler;
    sigemptyset(&(act_int.sa_mask));
    act_int.sa_flags = 0;

    if (sigaction(SIGINT, &act_int, NULL) < 0) // captura de señal SIGINT
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// COMPROBADOR /////////////////////////////////////////////////////////////////////

/**
 * @brief Obtenemos un bloque de minero, comprobamos si ese correcto y pasamos este resultado a monitor
 *
 * @param lag Tiempo en milisegundos entre comprobacion y comprobacion
 * @param fd Fichero donde se encuentra la memoria compartida
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int comprobador(int fd_shm)
{
    Block receive;
    mqd_t mq;
    struct timespec ts;
    struct sigaction act_int;
    int sval;

    // Capturamos la señal SIGINT
    auxHandlerSI(act_int);

    shmmtc = mmap(NULL, sizeof(shmmtc), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (shmmtc == MAP_FAILED)
    {
        return EXIT_FAILURE;
    }
    // Creamos la cola de mensajes
    mq = mq_open(MQ_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, &attributes); // Abrimos la cola de mensajes
    if (mq == (mqd_t)-1)
    {
        munmap(&shmmtc, sizeof(shmmtc)); // Borramos el area mapeada de memoria compartida
        fprintf(stderr, "Error opening the queue\n");
        return EXIT_FAILURE;
    }

    while (TRUE)
    {
        printf("Llego aqui\n");
        // Obtenemos el mensaje de Minero
        if (mq_receive(mq, (char *)&receive, sizeof(Block), NULL) == -1)
        {
            fprintf(stderr, "Error receiving message\n");
            munmap(&shmmtc, sizeof(shmmtc)); // Borramos el area mapeada de memoria compartida
            mq_close(mq);
            mq_unlink(MQ_NAME);
            sem_destroy(&(shmmtc->mutex));     // Borramos el semaforo sem_empty
            sem_destroy(&(shmmtc->sem_fill));  // Borramos el semaforo sem_fill
            sem_destroy(&(shmmtc->sem_empty)); // Borramos el semaforo mutex
            return EXIT_FAILURE;
        }
        printf("Y a aqui\n");

        // Comprobamos que el valor del minero es el correcto
        if (receive.target == pow_hash(receive.solution))
        {
            receive.flag = TRUE;
        }
        else
        {
            receive.flag = FALSE;
        }

        sem_wait(&(shmmtc->sem_empty)); // Iniciamos parte Productor de Productor/Consumidor
        sem_wait(&(shmmtc->mutex));
        printf("Se lo envio a monitor\n");
        shmmtc->shminfo = &receive;
        shmmtc->correcto = 1;
        shmmtc->finalizando = 0;
        sem_post(&(shmmtc->mutex));
        sem_post(&(shmmtc->sem_fill)); // Terminamos parte Consumidor de Productor/Consumidor

        if (receive.flag == FALSE)
            break;
    }

    munmap(&shmmtc, sizeof(shmmtc)); // Borramos el area mapeada de memoria compartida
    mq_close(mq);
    mq_unlink(MQ_NAME);
    sem_destroy(&(shmmtc->mutex));     // Borramos el semaforo sem_empty
    sem_destroy(&(shmmtc->sem_fill));  // Borramos el semaforo sem_fill
    sem_destroy(&(shmmtc->sem_empty)); // Borramos el semaforo mutex
    return EXIT_SUCCESS;
}