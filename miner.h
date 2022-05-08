/**
 * @file miner.h
 * @brief Estructuras necesarias para minero y funcion minero
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */
#ifndef _MINER_H
#define _MINER_H

#include <semaphore.h>

#define MAX_MINERS 100
#define MAX_WORDS 400
#define MAX_AUX 32

#define SHM_NAME "/shm_minero"


int miner(int rounds, int n_threads, int fd, int fd_shm);



//Enumeracion boolean
typedef enum {
    FALSE, 
    TRUE
}Boolean;

//Identificador de cada MINERO
typedef struct {
    int pid;                            //PID del proceso
    int coins;                          //Monedas en la cartera
}Wallet;

//Bloque Memoria compartida MINEROS
typedef struct {
    int id;                             //Id del bloque
    long int target;                    //Numero a encontrar
    long int solution;                  //Posicion del numero encontrado
    int pidwinner;                      //Proceso ganador
    Wallet *wallets[MAX_MINERS];        //Carteras de los procesos
    int tvotes;                         //Todos los votos
    int pvotes;                         //Votos positivos
    Boolean flag;                       //Bandera
} Block;

//
typedef struct
{
    int minersvoting;
    pid_t pids_esperando[MAX_MINERS];   //Procesos que estan esperando
    pid_t pids_minando[MAX_MINERS];     //Procesos que estan minando
    int votes[MAX_MINERS];              //Votos de los mineros
    Wallet *wallets[MAX_MINERS];        //Cartera de los procesos
    Block prevblock;                    //Bloque viejo
    Block newblock;                     //Bloque nuevo
    sem_t sem_miners;                   //Semaforo para limitar mineros
    sem_t sem_waiting;                  //Semaforos que esperan a la señal SIGUSR1
    sem_t mutex;
}SHM_info;

typedef struct
{
    SHM_info *shminfo;
    int correcto;
    int finalizando;
    sem_t mutex;
    sem_t sem_fill;
    sem_t sem_empty;
}SHM_mtc;

#endif