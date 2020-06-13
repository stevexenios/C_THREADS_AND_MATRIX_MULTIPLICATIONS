// EXTRA CREDIT FEATURES: EC1, EC2 and EC3 implemented 

/* Contributors: Steve Mwangi, Quoc Phung
 * Start Date: 05/25/20 
 * Assignment 2 - TCSS 422
 * Professor Wes Lloyd
 */

/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 * 
 *  Producer consumer bounded buffer program to produce random matrices in parallel
 *  and consume them while searching for valid pairs for matrix multiplication.
 *  Matrix multiplication requires the first matrix column count equal the 
 *  second matrix row count.  
 *
 *  A matrix is consumed from the bounded buffer.  Then matrices are consumed
 *  from the bounded buffer, one at a time, until an eligible matrix for multiplication
 *  is found.
 *
 *  Totals are tracked using the ProdConsStats Struct for:
 *  - the total number of matrices multiplied (multtotal from consumer threads)
 *  - the total number of matrices produced (matrixtotal from producer threads)
 *  - the total number of matrices consumed (matrixtotal from consumer threads)
 *  - the sum of all elements of all matrices produced and consumed (sumtotal from producer and consumer threads)
 *
 *  Correct programs will produce and consume the same number of matrices, and
 *  report the same sum for all matrix elements produced and consumed. 
 *
 *  For matrix multiplication only ~25% may be e
 *  and consume matrices.  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"

/**
 * Task 5 – Create one producer pthread and one consumer pthread in pcmatrix.c
 * to launch the parallel matrix production and multiplication.
 * 
 * Tasks 6- Once a 1 producer and 1 consumer version of the program is working 
 * correctly without deadlock, refactor pcmatrix.c to use an array of producer 
 * threads, and an array of consumer threads. The array size is NUMWORK. 
 * 
 * (Extra credit for correct implementation of 3 or more producer/consumer pthreads). 
*/
int main(int argc, char *argv[])
{
    // Process command line arguments
  int numw = NUMWORK;
  if (argc==1)
  {
    BOUNDED_BUFFER_SIZE=MAX;
    NUMBER_OF_MATRICES=LOOPS;
    MATRIX_MODE=DEFAULT_MATRIX_MODE;
    printf("USING DEFAULTS: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  else
  {
    if (argc==2)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=MAX;
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==3)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==4)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==5)
    {
      numw=atoi(argv[1]); 
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=atoi(argv[4]);
    }
    printf("USING: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  bigmatrix = (Matrix **) malloc(sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);

  time_t t;
  
  // Seed the random number generator with the system time
  srand((unsigned) time(&t));
  
  // DELETED...
  printf("Producing %d matrices in mode %d.\n",NUMBER_OF_MATRICES,MATRIX_MODE);
  printf("Using a shared buffer of size=%d\n", BOUNDED_BUFFER_SIZE);
  printf("With %d producer and consumer thread(s).\n",numw);
  printf("\n");

  // Task 6
  pthread_t pr[numw];
  pthread_t co[numw];

  if(numw < 1){
    printf("Invalid Value for numw = %d.\n", numw);
    return 0;
  }

  if(BOUNDED_BUFFER_SIZE < 1){
    printf("Invalid Value for BOUNDED_BUFFER_SIZE = %d.\n", BOUNDED_BUFFER_SIZE);
    return 0;
  }
  
  // consume ProdConsStats from producer and consumer threads
  ProdConsStats * produced_matrices = (ProdConsStats *) malloc(sizeof(ProdConsStats));
  produced_matrices->multtotal = 0;
  produced_matrices->matrixtotal = 0;
  produced_matrices->sumtotal = 0;

  ProdConsStats * consumed_matrices = (ProdConsStats *) malloc(sizeof(ProdConsStats));
  consumed_matrices->multtotal = 0;
  consumed_matrices->matrixtotal = 0;
  consumed_matrices->sumtotal = 0;
  
  /**
   * Iteratively creating 2 threads(p&c) and joining them
   * _REFERENCES: Stack_overflow since we could not track some
   * of the errors in the get and put routines directly even though
   * it was working. So alternative was to manually track using the
   * print f.
  */
  for (int i = 0; i < numw; i++){
    if (pthread_create(&pr[i], NULL, prod_worker, produced_matrices) != 0){
      printf("Producer thread #%d creation failed.\n", i);
    }
    if (pthread_create(&co[i], NULL, cons_worker, consumed_matrices) != 0){
      printf("Consumer thread #%d creation failed.\n", i);
    }
  }

  for (int i = 0; i < numw; i++){
    pthread_join(pr[i], NULL);
    pthread_join(co[i], NULL);
  }

  // add up total matrix stats in prs, cos, prodtot, constot, consmul 
  int prs = produced_matrices->sumtotal;
  int cos = consumed_matrices->sumtotal;
  int prodtot = produced_matrices->matrixtotal;
  int constot = consumed_matrices->matrixtotal;
  int consmul = consumed_matrices->multtotal;

  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n",prs,cos);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n",prodtot,constot,consmul);
  free(bigmatrix);
  free(produced_matrices);
  free(consumed_matrices);
  return 0;
}
