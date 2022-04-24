/**
 * @file pow.h
 * @author SOPER teaching team.
 * @author Rafael Domínguez Sáez
 * @author Laura M García Suárez
 * @brief Computation of the POW.
 * @version 1.0
 * @date 2022-04-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _POW_H
#define _POW_H

#define POW_LIMIT 99997669 /*!< Maximum number for the hash result. */

/**
 * @brief Computes the following hash function:
 * f(x) = (X * x + Y) % P.
 * 
 * @author SOPER teaching team.
 *
 * @param x Argument of the hash function, x.
 * @return Result of the hash function, f(x).
 */
long int pow_hash(long int x);

/**
 * @brief Computes the following hash function:
 * f(x) = (X * x + Y + 1) % P.
 * ESTA FUNCION ES PARA COMPROBAR LA SALIDA DEL PROGRAMA CUANDO DA ERROR
 * 
 * @author Laura M García Suárez
 * @author Rafael Domínguez Sáez
 *
 * @param x Argument of the hash function, x.
 * @return Result of the hash function, f(x).
 */
long int pow_hash2(long int x);

#endif
