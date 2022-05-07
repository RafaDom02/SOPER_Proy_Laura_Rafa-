/**
 * @brief Comprobador: 
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// COMPROBADOR /////////////////////////////////////////////////////////////////////


/**
 * @brief Obtenemos un bloque de minero, comprobamos si ese correcto y pasamos este resultado a monitor
 * 
 * @param lag Tiempo en milisegundos entre comprobacion y comprobacion
 * @param fd Fichero donde se encuentra la memoria compartida
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int comprobador(int fd)
{
    SHM_info shminfo;
    mqd_t mq;
    struct timespec ts;
    int sval;

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