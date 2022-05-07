/**
 * @brief Comprobador: obtiene un bloque de minero, comprueba el resultado
 *        y este resultado se lo pasa a monitor
 *  
 * @author Rafael Domínguez Saez rafael.dominguez@estudiante.uam.es
 * @author Laura María García Suárez lauramaria.garcias@estudiante.uam.es
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// BIBLIOTECAS /////////////////////////////////////////////////////////////////////
#include <stdio.h>          //Biblioteca base
#include <stdlib.h>         //Biblioteca para funciones adicionales
#include <time.h>           //Biblioteca para nanosleep()
#include <sys/mman.h>       //Biblioteca para mmap()
#include <unistd.h>         //Biblioteca para ftruncate()
#include <sys/stat.h>       //Biblioteca para sem_open()
#include <fcntl.h>          //Biblioteca para sem_open()
#include <sys/types.h>      //Biblioteca para sem_wait()

#include "comprobador.h"    //Biblioteca de comprobador
#include "pow.h"            //Biblioteca para hash_pow()
#include "miner.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// COMPROBADOR /////////////////////////////////////////////////////////////////////
/**
 * @brief Inicializamos los semaforos sem_empty, sem_fill y mutex
 * 
 * @param sem_empty Semaphore
 * @param sem_fill Semaphore
 * @param mutex Semaphore
 * @return EXIT_FAILURE or EXIT_SUCCCESS
 */
int crearSemaforos(sem_t *sem_empty, sem_t *sem_fill, sem_t *mutex)
{
    // Creacion del primer semaforo
    if (sem_init(sem_empty, 1, MAX_BUFFER) == -1)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Creacion del segundo semaforo
    if (sem_init(sem_fill, 1, 0) == -1)
    {
        sem_destroy(sem_empty);
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // Creacion del mutex
    if (sem_init(mutex, 1, 1) == -1)
    {
        sem_destroy(sem_empty);
        sem_destroy(sem_fill);
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Obtenemos un bloque de minero, comprobamos si ese correcto y pasamos este resultado a monitor
 * 
 * @param lag Tiempo en milisegundos entre comprobacion y comprobacion
 * @param fd Fichero donde se encuentra la memoria compartida
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int comprobador(int lag, int fd)
{
    SHM_info shminfo;
    mqd_t mq;
    struct timespec ts;
    int sval;
    ts.tv_sec = 0;
    ts.tv_nsec = lag * 1000000; // lag para la espera activa

    mq = mq_open(MQ_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, &attributes); //Abrimos la cola de mensajes
    if(mq == (mqd_t)-1) {
        fprintf(stderr, "Error opening the queue\n");
        return EXIT_FAILURE;
    }

    if (ftruncate(fd, sizeof(SHM_info)) == -1) //Truncamos el segmento de memoria compartida con espacio para Sem_Block
    {
        perror("ftruncate");
        return EXIT_FAILURE;
    }

    //Obtenemos el Sem_Block de la memoria compartida
    if ((shminfo = (SHM_info *)mmap(NULL, sizeof(SHM_info), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
    }

    
    }

    return EXIT_SUCCESS;
}