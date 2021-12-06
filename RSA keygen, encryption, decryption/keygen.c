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

#define OPTIONS "b:i:n:d:s:vh"

//  I've added this one goofy line to test github
//  added a second goofy line
//  more goofy lines

//static uint16_t unique_sym = 0;
//static int inindex=0;
//static uint8_t outbuffer[BLOCK];
//static uint8_t inbuffer[BLOCK];

Set flag_set = (Set) 0;

//  template for parsing code using getopt thanks to Eugene
//  getopt will parse the command line passed.
//  if no arguments, it will print out the help file else
//  -b: specifies the minimum bits needed for the public modulus n.
//  -i: specifies the number of Miller-Rabin iterations for testing primes (default:50)
//  -n: pbfile: specifies the public key file (default: rsa.pub)
//  -d: pvfile: specifies the private key file (default: rsa.priv)
//  -s:  specifies the random seed for the random state init (default: seconds since Unix epoch, NULL
//  -v: enable verbose output
//  -h displays program synopsis and usage
//


int
main (int argc, char *argv[])
{
  int iters = 50;
  int mod_bcnt = 250;
  char *username;

  uint64_t init_seed=0;
  uint64_t bitcnt;

  FILE *fp_pub, *fp_priv;
  char *pbfile = "rsa.pub";	//default file name
  char *pvfile = "rsa.priv";	//default priv key file name
  //    fp_pub=fopen(pbfile,"w");
  //    fprintf(fp_pub,"hello world");
  //    fclose(fp_pub); 


  //create needed gmp variables and init
  mpz_t test_no, p, q, n, d, e, s, sig_name;
  mpz_inits (test_no, p, q, n, d, e, s, sig_name, NULL);






  char *help_str = "SYNOPSIS\n"
    "Creates a public/private RSA key\n"
    "\n"
    "USAGE\n"
    "./keygen [-v] [-h] [-b] [-i] [-s] [-n rsa.pub] [-d rsa.priv]\n"
    "\n"
    "OPTIONS\n"
    "-b	     specifies min bits for public mod n"
    "-i	     number of Miller_Rabin iterations (default: 50)"
    "-n pbfile	     public key file (default: rsa.pub)"
    "-d pvfile	     private key file (default: rsa.priv)"
    "-s	     seed for random state.  devault is NULL for Unix epoch time"
    "-v             verbose output\n"
    "-h             Program usage and help.\n";


  int opterr;			//supress printing an error message for badly form args
  int opt = 0;

//  Set up command line processing variable below
//
  opterr = 0;

//    int fdout = fileno(stdout);
//    int fdin = fileno(stdin);
//    char *file_name=optarg;

  //
  //  Command line parsing is below.  Keep track of which switches and arguments
  //  where passed.  Change default value for 'directed' graph, input and
  //  output files if specified.
  //

  while ((opt = getopt (argc, argv, OPTIONS)) != -1)
    {
      switch (opt)
	{
	case 'v':		// Set the Verbose switch - checked on printing
	  flag_set = insert_set (0, flag_set);
	  break;
	case 'b':
	  flag_set = insert_set (1, flag_set);
	  mod_bcnt = atoi (optarg);
	  break;
	case 'i':
	  flag_set = insert_set (2, flag_set);
	  iters = atoi (optarg);
	  break;

	case 'n':		// Set the input file name
	  flag_set = insert_set (3, flag_set);
	  pbfile = optarg;

	  break;
	case 'd':
	  flag_set = insert_set (4, flag_set);
	  pvfile = optarg;
	  break;
	case 's':
	  flag_set = insert_set (5, flag_set);
	  init_seed = atoi (optarg);
	  break;
	case 'h':
	  flag_set = insert_set (8, flag_set);
	  printf ("%s\n", help_str);
	  exit (1);

	default:
	  printf ("%s\n", help_str);
	  exit (1);
	}
    }
  //open the public and private key files to write to.
  //set mode on the private key file to 0600
  fp_pub = fopen (pbfile, "w");
  fp_priv = fopen (pvfile, "w");

  // stream_no= fileno(pf_priv);
  //fchmod(stream_no,x0600);

  uint64_t seed = time (NULL);	//  set to the epoch of time (fun to say)
  if (member_set (5, flag_set))
    {				// change the see from the epoc
      seed = init_seed;
    }

  randstate_init (seed);
//        printf("just initialized random generator\n");


  username = getenv ("USER");
  if (username == NULL)
    username = "NULL username";
  mpz_set_str (sig_name, username, 62);


  // make the public and private keys
  rsa_make_pub (p, q, n, e, mod_bcnt, iters);
  rsa_make_priv (d, e, p, q);

  // Produce signature

  rsa_sign (s, sig_name, d, n);

/*  
  if (rsa_verify(sig_name,s,e,n)){
      printf("signature matches\n");
  }
  else printf("signature don't match.\n");
*/

  // now print them if the -v option is set
  if (member_set (0, flag_set))
    {				// verbose mode set
      printf ("user = %s\n", username);
      bitcnt = mpz_sizeinbase (s, 2);
      gmp_printf ("s (%" PRIu64 " bits) = %#8Zx\n", bitcnt, s);
      bitcnt = mpz_sizeinbase (p, 2);
      gmp_printf ("p (%" PRIu64 " bits) = %#8Zx\n", bitcnt, p);
      bitcnt = mpz_sizeinbase (q, 2);
      gmp_printf ("q (%" PRIu64 " bits) = %#8Zx\n", bitcnt, q);
      bitcnt = mpz_sizeinbase (n, 2);
      gmp_printf ("n (%" PRIu64 " bits) = %#8Zx\n", bitcnt, n);
      bitcnt = mpz_sizeinbase (e, 2);
      gmp_printf ("e (%" PRIu64 " bits) = %#8Zx\n", bitcnt, e);
      bitcnt = mpz_sizeinbase (d, 2);
      gmp_printf("d (%"PRIu64" bits) = %#8Zx\n",bitcnt,d);

    }

  rsa_write_pub (n, e, s, username, fp_pub);
  rsa_write_priv (n, d, fp_priv);

//         only for testing purpose.  Take out for final code.
//      
//      if (member_set(0, flag_set)) { //verbose mode set
//        bitcnt = mpz_sizeinbase(test_no,2);
//        gmp_printf("\n(%"PRIu64" bits) = %#8Zx\n %#8Zd\n",bitcnt,test_no,test_no);
//        int n = mpz_probab_prime_p(test_no,20);
//        printf("\nThe probability we have a prime is: %d\n",n);
//      }

  fclose (fp_pub);
  fclose (fp_priv);
  mpz_clears (test_no, p, q, n, d, e, s, sig_name, NULL);
  randstate_clear ();
//        printf("released randomized memory\n");
  return 0;

}
