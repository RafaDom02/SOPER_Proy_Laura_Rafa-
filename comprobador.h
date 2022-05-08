/**
 * @file comprobador.h
 *
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */

 #ifndef _COMPROBADOR_H
 #define _COMPROBADOR_H

 #include "miner.h"

 #define SHM_MTC_NAME "/shm_comprobador_monitor"

typedef struct
{
    Block *shminfo;
    int correcto;
    int finalizando;
    sem_t mutex;
    sem_t sem_fill;
    sem_t sem_empty;
}SHM_mtc;

int comprobador(int fd_shm);

#endif 