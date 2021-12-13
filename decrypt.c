#define _GNU_SOURCE

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <gmp.h>


#include <time.h>
#include "set.h"

#include "randstate.h"
#include "numtheory.h"
#include "rsa.h"

#define OPTIONS "i:o:n:vh"

Set flag_set = (Set) 0;
    
//  template for parsing code using getopt thanks to Eugene
//  getopt will parse the command line passed.
//  if no arguments, it will print out the help file else
//  -i: specifies the number of Miller-Rabin iterations for testing primes (default:50)
//  -n: pbfile: specifies the public key file (default: rsa.pub)
//  -d: pvfile: specifies the private key file (default: rsa.priv)
//  -s:  specifies the random seed for the random state init (default: seconds since Unix epoch, NULL
//  -v: enable verbose output
//  -h displays program synopsis and usage
//


int main (int argc, char*argv[])
	  {
	  uint64_t bitcnt;
	 
	 FILE *fp_priv, *fp_input, *fp_output;
	 char *encfile_i = "stdin";              //defaults to stdin
	 char *encfile_o = "stdout";            //defaults to stdoout
     char *pvfile = "rsa.priv";     //default priv key file name
 


	  //create needed gmp variables and init
	  mpz_t test_no,p,q,n,d,e,s,sig_name;
	  mpz_inits(test_no,p,q,n,d,e,s,sig_name,NULL);






    char *help_str = "SYNOPSIS\n"
                     "Decrypts a file using your private rsa key\n"
                     "\n"
                     "USAGE\n"
		     "./keygen [-v] [-h] [-i input encrypt file] [-o output  file] [-n rsa.priv]\n"
                     "\n"
		     "OPTIONS\n"
		     "-i encfile_i   specifies encrypted input file (default: stdin)"
		     "-o encfile_o   specifies the decrypted output file to write to (default: stdout)"
		     "-n pvfile	     private key file (default: rsa.pub)"
		     "-v             verbose output\n"
		     "-h             Program usage and help.\n";


    int opterr; //supress printing an error message for badly form args

    int opt = 0;

//  Set up command line processing variable below
//
    opterr = 0;

    //int encfile_o = fileno(stdout);
    //int encfile_i = fileno(stdin);
    
 //
 //  Command line parsing is below.  Keep track of which switches and arguments
 //  where passed.  Change default value for 'directed' graph, input and
 //  output files if specified.
 //

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v':				// Set the Verbose switch - checked on printing
            flag_set = insert_set(0,flag_set);
            break;
	    case 'i':
	        flag_set = insert_set(2,flag_set);
	        encfile_i = optarg;
	        break;
	    case 'o':
	        flag_set = insert_set(1,flag_set);
	        encfile_o = optarg;
	        break;
        case 'n': 				// Set the private key file, defaults to rsa.priv
	        flag_set = insert_set(3, flag_set);
	        pvfile = optarg;
	        break;
        case 'h':
            flag_set = insert_set(8, flag_set);
            printf("%s\n", help_str);
            exit(1);
        default:
	        printf("%s\n", help_str);
	        exit(1);
        }
    }

    fp_priv = fopen(pvfile,"r");     // open private rsa key file
    fp_input = fopen(encfile_i,"r");
    fp_output = fopen(encfile_o,"w");
    
    
     rsa_read_priv(n,d,fp_priv);
     rsa_decrypt_file(fp_input,fp_output,n,d);
    

	
	// now print them if the -v option is set
	if (member_set(0, flag_set)) { // verbose mode set

	  bitcnt= mpz_sizeinbase(n,2);
	  gmp_printf("n (%"PRIu64" bits) = %#8Zx\n",bitcnt,n);
	  bitcnt= mpz_sizeinbase(d,2);
	  gmp_printf("d (%"PRIu64" bits) = %#8Zx\n",bitcnt,d);

	}  
    
	
	fclose(fp_priv);
	fclose(fp_input);
	fclose(fp_output);
	mpz_clears (test_no,p,q,n,d,e,s,sig_name,NULL);
//        printf("released randomized memory\n");
	  return 0;

}


