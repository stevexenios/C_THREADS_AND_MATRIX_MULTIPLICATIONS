/*
 *  Contributing Authors: 
 *  1. Steve Mwangi
 *  2. Quoc Phung
 * 
 *  TCSS 422 - Professor Wes Lloyd
 *  Assignment 2
 *  Spring 2020
 * 
 *  prodcons module
 *  Producer Consumer module
 * 
 *  Implements routines for the producer consumer module based on 
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"
// Header file with the pthreads
#include <sys/types.h>
// Header file for sys calls, but should we use unistd.h????
#include <sys/syscall.h>
#include <stdbool.h>

/**
 * We need two condition locks(Chapter 30 pg. 12):
 * 1. Producer waits on the condition empty, and signals fill
 * 2. Consumer waits on the condition fill, and signals wait
 * 3. Mutex lock is for the producer and consumer locking
 * 4. Locks for get and put routines
 * This prevents a consumer accidentally waking a producer,
 * and vice versa.
 * 
 * Declaring and initializing them
 */
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t routine_lock = PTHREAD_MUTEX_INITIALIZER; 

/**
 * Counter variables (See Chapter 30 page 13)
 * Bounded buffer bigmatrix defined in prodcons.h
 */
int fill_ptr = 0;
int use_ptr = 0;
int counter = 0; //Count number of matrices

/**
 * Task 1- Implement a bounded buffer. This will be a buffer of
 * pointers to Matrix structs (records). The datatype should be
 * “Matrix * bigmatrix[MAX]”, and the bounded buffer will be 
 * limited to MAX size. 
 * 
 * The buffer can then be initialized as follows where MAX is the size
 * The bounded buffer is a NULL terminated array of struct Matrix pointers of size
 * “BOUNDED_BUFFER_SIZE”: 
 */
//bigmatrix[MAX]; // Task 1-> see *.h; 

// TO DO
// Less heap allocation, valgrind is complaining

/**
 * Alternative data Structures 
 * 1. Prod_cons for counting produce/consumed matrices
 * 2. Matrices keep track of matrices
 */
counters_t prod_cons;
Matrix *first_matrix;
Matrix *second_matrix;
Matrix *product_matrix;

/**
 * Task 2 – Implement get() and put() routines for the bounded buffer.
 * Chapter 29-30 
*/
int put(Matrix *value){
  pthread_mutex_lock(&routine_lock);
  // ALternative way
  // increment(prod_cons->prod);
  if(value != NULL){
    if(counter < BOUNDED_BUFFER_SIZE){
      bigmatrix[fill_ptr] = value;
      use_ptr = fill_ptr;
      fill_ptr = (fill_ptr + 1) % BOUNDED_BUFFER_SIZE;
      counter++;
    }
  }
  pthread_mutex_unlock(&routine_lock);
}

/**
 * Ch 27 - 30
 * Task 2 – Implement get() and put() routines for the bounded buffer. 
*/
Matrix *get(){ // Tsak 2 
  // int get_count = get_cnt(prods_cons->cons) % MAX;
  // pthread_mutex_lock(&matrix_count->lock);
  Matrix * temp;
  pthread_mutex_lock(&routine_lock);
  if(counter > 0){
    temp = bigmatrix[use_ptr];
    fill_ptr = use_ptr;
    use_ptr = (use_ptr - 1) % BOUNDED_BUFFER_SIZE;
    counter--;
  }
  // increment_cnt(prods_cons_count->cons);
  // decrement_cnt(matrix_count); // 
  pthread_mutex_unlock(&routine_lock);
  return temp;
}

/**
 * Task 3 – Call put() from within prod_worker() and add all necessary 
 * uses of mutex locks, condition variables, and signals. 
 *
 * Integrate the counters. Calculate running total for produced matrices.
 * 
 * Based on Chapter 30 of book page 7 - 11
 * 
 */
void *prod_worker(void *arg) {
  ProdConsStats *produced_matrices = (ProdConsStats *) arg;
  for(int i = 0; i < LOOPS; i++){
    // Always hold lock when signaling
    // Using "while" loops instead of "if"
    // Purpose mesasemantics --> prevents non concurrent consumption with "if"  
    pthread_mutex_lock(&mutex);
    // Using "while" loops instead of "if"
    // Purpose mesasemantics
    // Prevents non-concurrent consumption
    while(counter == MAX){
      // Producer waits on condition empty
      pthread_cond_wait(&empty, &mutex);
    }
    // create random matrix
    Matrix *temp = GenMatrixRandom();
    put(temp);
    
    pthread_mutex_lock(&counter_lock);
    // increase matrix total & sum total
    produced_matrices->matrixtotal += 1;
    produced_matrices->sumtotal += SumMatrix(temp);
    pthread_mutex_unlock(&counter_lock);
    // Producer signals fill
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&mutex);
  }
}

/**
 * Helper function to evaluate if we have more matrices
 * produced compared to consumed
 */
// _Bool difference() {
//   int p1 = (int) (prod_cons->prod);
//   int p2 = (int) (prod_cons->cons);
//   return  (p1 - p2)  > 0 ? true: false;
// }

/**
 * Task 4 – Call get() from within cons_worker() and 
 * all necessary uses of mutex locks, condition variables, 
 * and signals. Integrate the counters. Implement the matrix 
 * multiplication by consuming matrices from the bounded buffer
 * as described above. Calculate running total for consumed matrices.
 * 
 * Based on Chapter 30 of book page 7-11
 *  
*/
void *cons_worker(void *arg){
  ProdConsStats *consumed_matrices = (ProdConsStats *) arg;
  for(int i = 0; i < LOOPS; i++){
    pthread_mutex_lock(&mutex);
    // Using "while" loops instead of "if"
    // Purpose mesasemantics
    // Prevents non-concurrent consumption
    while(counter == 0){
      // Broadcast to all waiting threas..
			//pthread_cond_broadcast(&empty);
      pthread_cond_wait(&fill, &mutex); // Consumer waits on condition fill
    }
    if(first_matrix == NULL){
      // Get Matrix to consume
      first_matrix = get();
      pthread_mutex_lock(&counter_lock);
      // increase number of matrix totals
      // Increas sum of values in the matrix by its sum vals
      consumed_matrices->matrixtotal++;
      consumed_matrices->sumtotal += SumMatrix(first_matrix);
      pthread_mutex_unlock(&counter_lock);
    }
    else if(second_matrix == NULL){
      // Get second Matrix to consume
      second_matrix = get();
      pthread_mutex_lock(&counter_lock);
      // increase number of matrix totals
      // Increas sum of values in the matrix by its sum vals
      consumed_matrices->matrixtotal++;
      consumed_matrices->sumtotal += SumMatrix(second_matrix);
      pthread_mutex_unlock(&counter_lock);
      // Otherwise, empty second_matrix
      if(MatrixMultiply(first_matrix, second_matrix) == NULL){
        pthread_mutex_lock(&counter_lock);
        pthread_mutex_unlock(&counter_lock);
        // free second_matrix & set to be empty
        FreeMatrix(second_matrix);
        second_matrix = NULL;
      }
    }
    // QP, could the if's be the ones causing the difference in conformancy
    // with the produced vs consumed, mesasemantics and what NOT? 
    if (first_matrix != NULL && second_matrix != NULL){
      // printf("MULTIPLYING (%d x %d) BY (%d x %d):\n", first_matrix->rows, first_matrix->cols, second_matrix->rows, second_matrix->cols);
      printf("MULTIPLY (%d x %d) BY (%d x %d):\n", first_matrix->rows, first_matrix->cols, second_matrix->rows, second_matrix->cols);
      DisplayMatrix(first_matrix, stdout);
      printf("    X\n");
      DisplayMatrix(second_matrix, stdout);
      // get result for product_matrix
      product_matrix = MatrixMultiply(first_matrix, second_matrix);
      pthread_mutex_lock(&counter_lock);
      //Increase total of multiply
      consumed_matrices->multtotal++;
      pthread_mutex_unlock(&counter_lock);
      printf("    =\n");
      // display product_matrix
      DisplayMatrix(product_matrix, stdout);
      // free all matrices
      FreeMatrix(product_matrix);
      FreeMatrix(second_matrix);
      FreeMatrix(first_matrix);
      // set all matrices to be empty
      // Inside the if so that only the time a valid multiplication is completed
      // We free up all matrices.
      // Set them up to NULL since the matrices are global, and they are "sticking around"
      first_matrix = NULL;
      second_matrix = NULL;
      product_matrix = NULL;
    }
    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
  }
}
