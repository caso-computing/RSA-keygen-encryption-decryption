
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

#include "randstate.h"
#include "numtheory.h"

//  added goofy line for source contrrol


void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters){
	mpz_t totient,p_minus1,q_minus1,grt_com_div;
	mpz_inits (totient,p_minus1,q_minus1,grt_com_div,NULL);
	mpz_set_ui(grt_com_div,0);
	
	uint64_t maxbits_p = ((3*nbits)/4)-1;
//	uint64_t minbits_p = nbits/4;
	
	//  find 1st prime p
    make_prime(p,maxbits_p, iters);
    
	//  find 2nd prime q
	uint64_t maxbits_q = nbits/4;
    make_prime(q,maxbits_q, iters);
	mpz_mul(n,p,q);		// n= p*q
	mpz_sub_ui(p_minus1,p,1);
	mpz_sub_ui(q_minus1,q,1);
	mpz_mul(totient,p_minus1,q_minus1);

	// next find a coprime number with the totient, then return that number as the pub exp
	while (mpz_cmp_ui(grt_com_div,1) != 0){
		mpz_urandomb(e,state,nbits);
		gcd(grt_com_div,e,totient);

	}
	
	mpz_clears(totient,p_minus1,q_minus1,grt_com_div, NULL);
       	return;
}

void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile){
    gmp_fprintf(pbfile,"%Zx\n",n);
    gmp_fprintf(pbfile,"%Zx\n",e);
    gmp_fprintf(pbfile,"%Zx\n",s);
    fprintf(pbfile,"%s\n",username);
}


void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile){
    char buff[100];
    gmp_fscanf(pbfile,"%Zx\n",n);
    gmp_printf("n = %#8Zx\n",n);
    gmp_fscanf(pbfile,"%Zx\n",e);
    gmp_fscanf(pbfile,"%Zx\n",s);
    gmp_printf("s = %#8Zx\n",s);
    gmp_fscanf(pbfile,"%[^\n] ",buff);
    gmp_printf("buff = %s\n", buff);
//    fscanf(pbfile,"%[^\n] ",buff);
//    printf("username is %s", buff);
}



void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q){
    mpz_t totient,rop,p_minus1,q_minus1;
    mpz_inits(totient,rop,p_minus1,q_minus1,NULL);
    
	mpz_sub_ui(p_minus1,p,1);
	mpz_sub_ui(q_minus1,q,1);
	mpz_mul(totient,p_minus1,q_minus1);    
    mod_inverse(d,e,totient);

    mpz_clears(totient,rop,p_minus1, q_minus1, NULL);
}



void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile){
    gmp_fprintf(pvfile,"%Zx\n",n);
    gmp_fprintf(pvfile,"%Zx\n",d);
}

void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile){
    uint64_t byte_read=gmp_fscanf(pvfile,"%Zx\n",n);
    byte_read = gmp_fscanf(pvfile,"%Zx\n",d);
}


// performs RSA encryption on the message block m (created in rsa_encrypt
// _file.  Returnes the encrypted cipher block in c
//
void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n){
	//c = (m^e)%n
	pow_mod(c,m,e,n);
	return;
}



void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e){
    mpz_t rop, c;
    mpz_inits(rop,c,NULL);

    uint64_t block_size = mpz_sizeinbase(n, 2);
    uint8_t *buffer = (uint8_t *)malloc(block_size*sizeof(uint8_t)); 
    //array[block_xize] of uint8_8 * bytes
    buffer[0]=0xff;         // set the first byte of the buffer to 0xff
    
    //read from infile and output to outfile
   uint64_t byte_read = fread(&buffer[1], 1, block_size-1, infile);
   mpz_import(rop,byte_read+1,1,1,1,0,buffer);  //import the block read into mpz type
   rsa_encrypt(c,rop,e,n);   //encrypt the block read
   gmp_fprintf(outfile,"%Zx\n",c);  //now write out the encrypted block in hexstring ening wiht \n
   
   while (byte_read==block_size-1) {
       byte_read = fread(&buffer[1], 1, block_size-1, infile);
       mpz_import(rop,byte_read+1,1,1,1,0,buffer);  //import the block read into mpz type
       rsa_encrypt(c,rop,e,n);   //encrypt the block read
       gmp_fprintf(outfile,"%Zx\n",c);  //now write out the encrypted block in hexstring ening wiht \n
   }
    
    mpz_clears(rop,c,NULL);
}


//  decrypt cipher c to message m using private key d and public modulus
//  n
//
void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n){
	// m=c^d % n
	pow_mod(m,c,d,n);
	return;
}


void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d){
    
    mpz_t rop, c;
    mpz_inits(rop,c,NULL);
    uint64_t bitcnt;
    unsigned long int byte_read;
    size_t block_size = mpz_sizeinbase(n,2);
    uint8_t *buffer = (uint8_t *)malloc(block_size*sizeof(uint8_t)); 
    //array[block_xize] of uint8_8 * bytes
 //   byte_read= gmp_fscanf(infile,"%Zx",c);
 //   bitcnt=mpz_sizeinbase(c,2);
 //   gmp_printf("c %llu bits) = %#8Zx\n",bitcnt,c);
    while (gmp_fscanf(infile,"%Zx",c)!=EOF){

        bitcnt= mpz_sizeinbase(c,2);
        gmp_printf("c %llu bits) = %#8Zx\n",bitcnt,c);
        rsa_decrypt(rop,c,d,n);
         bitcnt=mpz_sizeinbase(rop,2);
        gmp_printf("rop (%llu bits) = %#8Zx\n",bitcnt,rop);
        byte_read = bitcnt/8;
        mpz_export(buffer,&byte_read,1,1,1,0,rop);
        fwrite(&buffer[1],1,byte_read-1,outfile);
    }
    
    mpz_clears(rop,c,NULL);
}

 
void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n){
	pow_mod(s,m,d,n);
	return;
}


bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n){
	mpz_t v;
	mpz_init(v);
	pow_mod(v,s,e,n);
	if (mpz_cmp(v,m)==0) {
	    mpz_clear(v);
		return true;
	}
    mpz_clear(v);
	return false;
}
