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
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "miner.h"
#include "pow.h"
#include "mintocom.h"

/*****CONSTANTES DE ARGUMENTOS*****/
#define ARGS 3
#define MIN 0
#define MAX 1
#define NUM 2

/*****CONSTANTES DE MAXIMO DE SEGUNDOS PARA COMPROBAR LOS MINEROS DE LA VOTACION*****/
#define MAX_WAIT 5

/*****VARIABLES GLOBALES*****/
// Si se ha encontrado el número a buscar en la función hardwork()
SHM_info *shminfo = NULL;
int pos;
int n_th;
pthread_t *threads = NULL;
int fd_pipe;
mqd_t mq;

void *hardwork(void *param)
{
    long int i;
    int res;

    // Bucle desde el parametro min a max llamando a la funcion pow_hash()
    // donde comparamos todos los numeros del intervalo para encontrar el resultado deseado
    for (i = ((int *)param)[MIN]; i < ((int *)param)[MAX] && shminfo->newblock.solution == -1; i++)
    {
        res = pow_hash(i);
        if (res == ((int *)param)[NUM]) // Si encontramos la solucion
        {
            shminfo->newblock.pidwinner = getpid(); // guardamos el ganador en el bloque
            shminfo->newblock.solution = i;         // y guardamos la solucion
            return NULL;
        }
    }
    return NULL;
}

void returnAux()
{
    int val;
    if (threads)
        free(threads);
    mq_close(mq);
    if (shminfo)
    {
        sem_wait(&shminfo->mutex);
        shminfo->pids_minando[pos] = 0;
        shminfo->wallets[pos] = NULL;
        shminfo->newblock.wallets[pos] = NULL;
        sem_post(&(shminfo->sem_miners));
        sem_getvalue(&(shminfo->sem_miners), &val);
        sem_post(&shminfo->mutex);
        if (val == MAX_MINERS)
        {
            sem_destroy(&(shminfo->mutex));
            sem_destroy(&(shminfo->sem_miners));
            sem_destroy(&(shminfo->sem_waiting));
        }
        munmap(shminfo, sizeof(shminfo));
        if (val == MAX_MINERS)
        {
            shm_unlink(SHM_NAME);
        }
    }
    close(fd_pipe);
}

/**
 * @brief Se ejecuta cuando le llega la seña SIGUSR1
 *
 * @param sig Valor de la señal SIGUSR1 (no es usada)
 */
void handler_usr1(int sig) {}

/**
 * @brief Cuando llega la señal SIGINT
 *
 * @param sig Valor de la señal SIGINT
 */
void handler(int sig)
{
    returnAux();
    exit(EXIT_SUCCESS);
}

void voting(int sig)
{
    int i;
    int error;
    printf("OLAAAAA %d\n", getpid());
    sem_wait(&(shminfo)->mutex);
    shminfo->minersvoting++;
    sem_post(&(shminfo)->mutex);
    for (i = 0; i < n_th; i++)
    {
        printf("HolA\n");
        error = pthread_join(threads[i], NULL); // Esperamos a que todos los hilos terminen
        if (!error && error != ESRCH)
        {
            perror("pthread_join");
            sem_post(&shminfo->sem_miners);
            free(threads);
            exit(EXIT_FAILURE);
        }
    }
    printf("OLAAAAAA %d\n", getpid());
    if (getpid() == shminfo->newblock.pidwinner) // Voto del procesador ganador
        return;
    sem_wait(&(shminfo)->mutex); // Como vamos a escribir, nos aseguramos que solo escriba un proceso
    printf("OLAAAAAAA %d\n", getpid());
    if (pow_hash(shminfo->newblock.solution) == shminfo->newblock.target) // Comprobamos que la solucion esta correctamente
    {
        shminfo->votes[pos] = 1;    // Guardamos el voto
        shminfo->newblock.pvotes++; // Incrementamos los votos positivos
    }
    shminfo->newblock.tvotes++;
    sem_post(&(shminfo)->mutex);

    for (i = 0; i < MAX_WAIT; i++)
    {
        if (shminfo->minersvoting == shminfo->newblock.tvotes)
            break;
        sleep(1);
    }
    if (write(fd_pipe, shminfo, sizeof(shminfo)) == -1) // Enviamos por tuberia el mensaje a registrador
    {
        returnAux();
        perror("write");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Aplicacion de las señales al proceso
 *
 * @param act_usr1 puntero a estructura que define el comportamiento cuando se reciba la señal SIGUSR1
 */
void auxHandlerSG1(struct sigaction act_usr1)
{
    act_usr1.sa_handler = handler_usr1;
    sigemptyset(&(act_usr1.sa_mask));
    act_usr1.sa_flags = 0;

    if (sigaction(SIGUSR1, &act_usr1, NULL) < 0) // captura de señal SIGUSR1
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Aplicacion de las señales al proceso
 *
 * @param act_usr2 puntero a estructura que define el comportamiento cuando se reciba la señal SIGUSR2
 */
void auxHandlerSG2(struct sigaction act_usr2)
{
    // Creamos el handler para que se recoja la señal SIGUSR2
    act_usr2.sa_handler = voting;
    sigemptyset(&(act_usr2.sa_mask));
    act_usr2.sa_flags = 0;

    if (sigaction(SIGUSR2, &act_usr2, NULL) < 0) // captura de señal SIGUSR2
    {
        perror("sigaction");
        free(threads);
        exit(EXIT_FAILURE);
    }
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

void shm_next_round()
{
    int i;
    shminfo->prevblock = shminfo->newblock;
    shminfo->newblock.id = shminfo->prevblock.id + 1;
    shminfo->newblock.pvotes = 0;
    shminfo->newblock.tvotes = 0;
    shminfo->newblock.target = shminfo->prevblock.solution;
    shminfo->newblock.solution = -1;
    for (i = 0; i < MAX_MINERS; i++)
    {
        shminfo->newblock.wallets[i] = shminfo->prevblock.wallets[i];
    }
}

int miner(int rounds, int n_threads, int fd, int fd_shm)
{
    int numperthr = POW_LIMIT / n_threads;
    int i;
    int j;
    int k;
    int error;
    int sval;
    int yavotado = 0;
    char minerToRegister[MAX_WORDS] = "";
    int mitadvotos = 0;
    char cadenaAux[MAX_AUX] = "";
    int param[n_threads][ARGS];
    struct sigaction act_usr1;
    struct sigaction act_usr2;
    struct sigaction act_int;
    sigset_t sigset;
    Wallet wallet;
    // Comprobación de errores
    if (rounds <= 0 || n_threads <= 0)
        return EXIT_FAILURE;

    fd_pipe = fd;

    // Alocamos memoria para todos los hilos
    threads = (pthread_t *)malloc(n_threads * sizeof(threads));
    if (!threads)
        return EXIT_FAILURE;

    auxHandlerSI(act_int);
    n_th = n_threads;
    // Acceder a la memoria compartida
    shminfo = (SHM_info *)mmap(NULL, sizeof(shminfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    close(fd_shm);
    if (shminfo == MAP_FAILED)
    {
        perror("mmap");
        free(threads);
        exit(EXIT_FAILURE);
    }
    // Si el sistema esta lleno, salimos
    if (sem_trywait(&(shminfo->sem_miners)) == -1)
    {
        munmap(shminfo, sizeof(shminfo));
        free(threads);
        return EXIT_SUCCESS;
    }
    if (getpid() != shminfo->prevblock.pidwinner)
    {
        sem_wait(&shminfo->mutex); // Usamos el mutex para asegurar que solo escriba 1 proceso
        for (i = 0; i < MAX_MINERS; i++)
        { // Buscamos sitio donde meter el pid del proceso en
            if (shminfo->pids_esperando[i] == 0)
            { // la memoria compartida
                shminfo->pids_esperando[i] = getpid();
                break;
            }
        }
        sem_post(&shminfo->mutex);
    }
    /////////////////////////////aqui///////////////////////////
    // Creamos la cola de mensajes
    if ((mq = mq_open(MQ_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attributes)) == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    // Para usar a la hora de que queramos que los procesos esperen a SIGUSR1
    sigfillset(&sigset);
    sigdelset(&sigset, SIGUSR1);
    auxHandlerSG1(act_usr1);
    // Para que los procesos esperen la señal SIGUSR1
    if (getpid() != shminfo->prevblock.pidwinner)
    {
        sigsuspend(&sigset);
    }
    // Para inicializar las carteras de todos los procesos
    wallet.coins = 0;
    wallet.pid = getpid();
    sem_wait(&(shminfo)->mutex);
    for (i = 0; i < MAX_MINERS; i++)
    {
        if (shminfo->newblock.wallets[i] == NULL)
        {
            shminfo->pids_minando[i] = getpid();
            shminfo->wallets[i] = &wallet;
            shminfo->newblock.wallets[i] = &wallet;
            pos = i;
            break;
        }
    }
    sem_post(&(shminfo)->mutex); //
    auxHandlerSG2(act_usr2);
    // Bucle donde nos aseguramos si se ha encontrado el número y hacemos que haga todas las rondas
    for (j = 0; j < rounds; j++)
    {
        if (shminfo->prevblock.pidwinner != getpid())
            sem_wait(&(shminfo->sem_waiting));
        else
        {
            if (j == 0)
            {
                for (i = 0; i < MAX_MINERS; i++)
                {
                    if (shminfo->pids_esperando[i] != 0)
                    {
                        kill(shminfo->pids_esperando[i], SIGUSR1);
                    }
                }
            }
        }
        // Inicializamos por primera vez param asignando el target inicial
        for (i = 0; i < n_threads; i++)
            param[i][NUM] = shminfo->newblock.target;
        // Por cada hilo le asignamos el intervalo de numeros que tienen que buscar y lo
        // creamos llamando a la funcion hardwork() con los parametros correspondientes
        for (i = 0; i < n_threads; i++)
        {
            param[i][MIN] = numperthr * i;
            if (i == n_threads - 1)
            {
                param[i][MAX] = POW_LIMIT; // dividimos los valores
            }
            else
            {
                param[i][MAX] = numperthr * (i + 1) - 1;
            }
            error = pthread_create(&threads[i], NULL, hardwork, param[i]); // Creamos los hilos para que vayan buscando
            if (error != 0)
            {
                returnAux();
                fprintf(stderr, "pthread_create: %s\n", strerror(error));
                free(threads);
                return EXIT_FAILURE;
            }
        }
        // esperamos el retorno de todos los hilos
        for (i = 0; i < n_threads; i++)
        {
            error = pthread_join(threads[i], NULL); // esperamos a que terminen los hilos
            if (error != 0)
            {
                returnAux();
                fprintf(stderr, "pthread_join: %s\n", strerror(error));
                free(threads);
                return EXIT_FAILURE;
            }
        }
        // si se encuentra y el numero de rondas ya ha terminado salimos de la función
        if (shminfo->newblock.pidwinner == getpid())
        {
            sem_wait(&(shminfo)->mutex); // descendemos el mutex
            shminfo->newblock.pvotes++;  // añadimos los votos
            shminfo->newblock.tvotes++;
            shminfo->minersvoting++;
            i = 0;
            //
            for (i = 0; i < MAX_WAIT; i++)
            {
                if (shminfo->minersvoting == shminfo->newblock.tvotes)
                    break;
                sleep(1);
            }
            // Si la mayoria de los votos es positivo, los mineros obtienen 1 moneda
            if (shminfo->newblock.tvotes / 2 < shminfo->newblock.pvotes)
            {
                shminfo->newblock.wallets[pos]->coins++;
            }
            // Concatenamos todos los datos que vamos a pasar a registrador
            sprintf(cadenaAux, "Id:             %d\n", shminfo->newblock.id);
            strcat(minerToRegister, cadenaAux);
            sprintf(cadenaAux, "Winner:         %d\n", shminfo->newblock.pidwinner);
            strcat(minerToRegister, cadenaAux);
            sprintf(cadenaAux, "Target:         %ld\n", shminfo->newblock.target);
            strcat(minerToRegister, cadenaAux);
            mitadvotos = shminfo->newblock.tvotes / 2;
            if (mitadvotos < shminfo->newblock.pvotes)
            {
                sprintf(cadenaAux, "Solution:       %ld (validated) \n", shminfo->newblock.solution);
                strcat(minerToRegister, cadenaAux);
            }
            else
            {
                sprintf(cadenaAux, "Solution:       %ld (rejected)\n", shminfo->newblock.solution);
                strcat(minerToRegister, cadenaAux);
            }
            sprintf(cadenaAux, "Votes:          %d/%d\n", shminfo->newblock.pvotes, shminfo->newblock.tvotes);
            strcat(minerToRegister, cadenaAux);
            sprintf(cadenaAux, "Wallets:        ");
            strcat(minerToRegister, cadenaAux);
            for (i = 0; i < MAX_MINERS; i++)
            {
                if (shminfo->newblock.wallets[i] != NULL)
                {
                    sprintf(cadenaAux, "%d:%d ", shminfo->newblock.wallets[i]->pid, shminfo->newblock.wallets[i]->coins);
                    strcat(minerToRegister, cadenaAux);
                }
            }
            shminfo->minersvoting = 0;
            // Enviamos la informacion obtenida a Registrador
            if (write(fd_pipe, minerToRegister, MAX_WORDS + 1) == -1)
            {
                returnAux();
                perror("write");
                exit(EXIT_FAILURE);
            }
            // Al guardar el numero que buscamos y su posicion, lo enviamos a la cola de mensajes
            if (mq_send(mq, (char *)&(shminfo->newblock), sizeof(Block), 1) == -1)
            {
                fprintf(stderr, "Error sending message\n");
                return EXIT_FAILURE;
            }
            printf("%s\n", minerToRegister);
            strcpy(minerToRegister, "");
            sem_post(&(shminfo)->mutex); // Habilitamos de nuevo el mutex
            shm_next_round();            // Preparamos a los procesos para la nueva ronda
            sem_wait(&(shminfo)->mutex); // Usamos de nuevo el mutex
            for (i = 0; i < MAX_MINERS; i++)
            {
                if (shminfo->pids_esperando[i] != 0)
                {
                    kill(shminfo->pids_esperando[i], SIGUSR1);
                    shminfo->pids_esperando[i] == 0;
                }
            }
            sem_post(&(shminfo)->mutex); // Activamos de nuevo el mutex
            sleep(1);
            sem_wait(&(shminfo)->mutex); // Utilizamos de nuevo el mutex
            for (i = 0; i < MAX_MINERS; i++)
            {
                if (shminfo->pids_minando[i] == getpid()) // Si el pid encuentra su pid no hace nada
                    continue;
                if (shminfo->pids_minando[i] != 0) // El pid manda a los demas pids a votar
                {
                    kill(shminfo->pids_minando[i], SIGUSR2); // Enviamos la señal a los procesos
                }
            }
            sem_post(&(shminfo)->mutex); // Habilitamos uan vez mas el mutex
        }
        else
        {
            while (TRUE)
            {
                if (yavotado == 1)
                    break;
            }
            yavotado = 0;
        }
    }
    //Si era la ultima ronda y justo gana enviará la señal para comenzar la siguiente ronda al resto
    if (shminfo->prevblock.pidwinner == getpid())
    {
        sem_wait(&(shminfo)->mutex);
        for (i = 0; i < MAX_MINERS; i++)
        {
            if (shminfo->pids_minando[i] != 0)
            {
                kill(shminfo->pids_minando[i], SIGUSR1);
            }
        }
        sem_post(&(shminfo)->mutex);
    }
    returnAux();
    return EXIT_SUCCESS;
}