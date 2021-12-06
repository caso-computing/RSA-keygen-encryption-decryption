


#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>

#include <time.h>


extern gmp_randstate_t state;

gmp_randstate_t state;


// use gmp to compute the greatest common denominator of two mp integers, a & b
//  return the gcd in g.  All three variables need to be initialized 1st via gmp
//  else you'll have unpleasant side effects.
//  Computes the gcd using the algorithm provided by prof long.
//

void gcd (mpz_t g, mpz_t a, mpz_t b)
{
  mpz_t temp, save_b,save_e;
  mpz_inits (temp,save_b,save_e,NULL);
  mpz_set(save_b,b);
  mpz_set(save_e,a);
  while ((mpz_sgn (b) != 0))
    {

      mpz_set (temp, b);	// temp = b
      mpz_mod (b, a, b);	// b= a%b
      mpz_set (a, temp);	// a=temp
    }
  mpz_set (g, a);		// g=a
  mpz_set(b,save_b);
  mpz_set(a,save_e);
  
  mpz_clears(temp,save_b,save_e, NULL);
  return;
}


void mod_inverse (mpz_t o, mpz_t a, mpz_t n)
{
  // temp1 = n=r, temp2=a=r'
  // o = t, o2 = t'
  mpz_t temp1, temp2, o2, q, temp, otemp;
  mpz_init (q);
  mpz_init (temp);
  mpz_init (otemp);
  mpz_init_set (temp1, n);
  mpz_init_set (temp2, a);
  mpz_init_set_ui (o, 0);
  mpz_init_set_ui (o2, 1);
  while (mpz_sgn (temp2) != 0)

    {
      mpz_fdiv_q (q, temp1, temp2);
      mpz_set (temp, temp2);
      mpz_mul (temp2, q, temp2);
      mpz_sub (temp2, temp1, temp2);
      mpz_set (temp1, temp);

      mpz_set (temp, o2);
      mpz_mul (o2, q, o2);
      mpz_sub (o2, o, o2);
      mpz_set (o, temp);
    }
  if (mpz_cmp_d (temp1, 1) > 0)
    {				// true if there is no mod inverse
      mpz_set_d (o, 0);
      return;
    }
  if (mpz_sgn (o) < 0)
    {
      mpz_add (o, o, n);
    }
  return;
}



//  this function computes the base 'a' raised to the 'd' power modulo n
//
void pow_mod (mpz_t o, mpz_t a, mpz_t d, mpz_t n)
{
  mpz_t p, g, result, two_const;
  mpz_inits (p,g, result, two_const, NULL);
  mpz_set (p, a);
  mpz_set_ui (o, (uint64_t) 1);		//set output =1
  mpz_set (g,d);		//set the exp to g
  mpz_set_ui (two_const, 2);  
  
  while (mpz_sgn (g) > 0)
    {
      if (mpz_odd_p(g) != 0){  //test to see if the exp is odd
	    mpz_mul (o, o, p); 
	    mpz_mod (o, o, n);
	  }
	 mpz_mul (p, p, p); 
	 mpz_mod (p, p, n); 
	 mpz_fdiv_q (g, g, two_const);
	}
	return;
}



// The following code was derived from python code contributed by mits
//

// n is the prime number to test and see if it is "probably" a prime-number.  The more times
// you "iters" through this calculation and can still return true, the higher chance you have
// found a prime number.
//
// iters is the number of times to test for primeness.
//  return true;
//  This function is called for all iter trials.
//  It returns false if n is composite and false
//  if n is probably prime.  
//  d (exponent) is an odd number such that n-1=r2^d 
//  for some r>=1
//
bool miller_rabin(mpz_t d, mpz_t n){
    mpz_t base, tstPrime, nminus1, temp_mod, square;
    mpz_inits(base, tstPrime, temp_mod, square, NULL);
    mpz_set_ui(tstPrime,1);
    mpz_set_ui(square,2);
    mpz_init_set(nminus1, n);
    mpz_sub_ui(nminus1, nminus1, 1);    // nminus1 = n-1
    
    mpz_sub_ui(temp_mod, n, 4);     // temp_mod = n-4
    mpz_urandomm(base,state,temp_mod);  //pick random base[0..n-4]
    mpz_add_ui(base,base,2);            // normalize base [2..n-4]
    pow_mod(tstPrime,base,d,n);         // returns tstPrime=(base^d)%n
   // gmp_printf("tstPrime =%#8Zx\n",tstPrime);
    if (mpz_cmp_d(tstPrime,1)==0 || mpz_cmp(tstPrime, nminus1)==0){
        return true;                    // any case we have a likely prime
    }
    while (mpz_cmp(d,nminus1) != 0){
        pow_mod(tstPrime,tstPrime, square, n);
      
        mpz_mul_ui(d,d,2);
        if (mpz_cmp_d(tstPrime,1)==0) {
            return false;
        }
        if (mpz_cmp(tstPrime,nminus1)==0){
            return true;
        }
    }
    return false;       //if you fall through while test we don't have prime
}
 
 


    // find r such that n = 2^d*r +1 for some r>=100
    



bool is_prime (mpz_t n, uint64_t iters)
	  {
	      if ((mpz_cmp_d(n,1)<=0) || (mpz_cmp_d(n,4)==0)){
	          return false;
	      }
	      if ((mpz_cmp_d(n,3))<=0){
	          return true;
	      }
	      
	      mpz_t d;          //  d will be the exponent
	      mpz_init(d);
	      mpz_sub_ui(d,n,1);        // d=n-1
	      //uint64_t nsize = mpz_sizeinbase(n,2);
	      while (mpz_scan0(d,0)==0){      //we have an even number.  make odd.
//	        gmp_printf("even =%#8Zx\n",d);
	          mpz_fdiv_q_ui(d,d,2);
	      }
	      for (uint64_t i=0; i<iters; i++){
	          if (!miller_rabin(d,n)){
	              return false;
	          }
	      }
	      return true;

}

void make_prime(mpz_t p, uint64_t bits, uint64_t iters){
    mpz_urandomb(p,state,bits);
    while(!(is_prime(p,iters))){
        mpz_urandomb(p, state, bits);
    }
	return;
}


