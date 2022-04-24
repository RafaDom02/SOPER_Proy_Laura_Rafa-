#include "pow.h"
#include <stdio.h>

#define PRIME POW_LIMIT
#define BIG_X 435679812
#define BIG_Y 100001819

long int pow_hash(long int x) {
    long int result = (x * BIG_X + BIG_Y) % PRIME;
    return result;
}

long int pow_hash2(long int x) {
    long int result = (x * BIG_X + BIG_Y + 1) % PRIME;
    return result;
}
