/**
 * @file miner.h
 *
 *
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 *
 */
#ifndef _MINER_H
#define _MINER_H

int miner(int rounds, int n_threads, int **fd, int fd_shm);

#define MAX_MINERS 100

//Enumeracion boolean
typedef enum {
    FALSE, 
    TRUE
}Boolean;

//Identificador de cada MINERO
typedef struct {
    int pid;
    int coins;
}Wallet;

//Bloque Memoria compartida MINEROS
typedef struct {
    int id;
    long int target;            //Numero a encontrar
    long int solution;          //Posicion del numero encontrado
    int pidw;
    Wallet wallets[MAX_MINERS];
    int tvotes;
    int pvotes;
    sem_t sem_miners;           //Semaforo para limitar mineros
    sem_t sem_waiting;          //Semaforos que esperan a la señal SIGUSR1
} Block;

//
typedef struct
{
    int pids[MAX_MINERS];
    int votes[MAX_MINERS];
    Wallet wallets[MAX_MINERS];
    Block prevblock;
    Block newblock;
}Pinfo;



#endif