
#include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <time.h>

#include "randstate.h"

extern gmp_randstate_t state;

gmp_randstate_t state;

void randstate_init(uint64_t seed){
	gmp_randinit_mt(state);
	gmp_randseed_ui(state, seed);
}

void randstate_clear(void){
	gmp_randclear(state);
	return;
}


