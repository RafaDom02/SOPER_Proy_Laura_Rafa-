/**
 * @file mintocom.h
 * @brief Estructura atributos
 *  
 * @date 2022-05-08
 *
 * @author Rafael Domínguez Saez rafael.dominguez@estudiante.uam.es
 * @author Laura María García Suárez lauramaria.garcias@estudiante.uam.es
 */
#ifndef _MINTOCOM_H
#define _MINTOCOM_H

#include <mqueue.h>
#include <semaphore.h>
#include "miner.h"

#define MQ_NAME "/mq_compr_miner"   //Nombre de la cola de mensajes

//Atributos para la cola de mensajes
struct mq_attr attributes = {
    .mq_flags = 0,
    .mq_maxmsg = 10,
    .mq_curmsgs = 0,
    .mq_msgsize = sizeof(Block)
};

#endif