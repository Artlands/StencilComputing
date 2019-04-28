/*
 * _PIMS_UTIL_C_  *
 * PIMS UTILITY FUNCTIONS
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "pims.h"

/* --------------------------------------------- READTRACE */

extern int read_trace( FILE *infile, trace_node *trace)
{
 /* vars */
 char buf[50];
 char *token;
 size_t len = 0;

 /* read a single entry from the trace file */
 if( feof(infile) ){
   return -2;
 }
 if( fgets( buf, 50, infile ) == NULL ){
   return -1;
 }

 /*
  * we have a valid buffer
  * strip the newline and tokenize it
  */

 len = strlen( buf );
 if( buf[len] == '\n' ){
   buf[len] = '\0';
 }

 // if( buf[0] == '#' ){
 //   // not a valid trace
 //   return -1;
 // }

 /* tokenize it */
 token = strtok( buf, ":");
 strcpy(trace->op, token);

 /* num_bytes */
 token = strtok( NULL, ":");
 trace->num_bytes = (int)(atoi(token));

 /* procid */
 token = strtok( NULL, ":");
 trace->procid = (int)(atoi(token));

 /* first part of address = 0x */
 token = strtok( NULL, "x");
 /* last part of address in hex */
 token = strtok( NULL, " ");
 trace->addr = (uint64_t)(strtol( token, NULL, 16 ));

 return 0;
}

/* EOF */

/* --------------------------------------------- WRITETOFILE */
extern void write_to_file(FILE* fp,
                          char* op,
                          int num_bytes,
                          int procid,
                          uint64_t addr)
{
  fprintf(fp, "%s:%d:%d:0x%016"PRIX64"\n", op, num_bytes, procid, addr);
}
/* EOF */

/* --------------------------------------------- GETPIMSID */

extern int getpimsid (uint32_t *pimsid,
                      uint64_t addr,
                      uint32_t shiftamt,
                      uint32_t num_vaults)
{
  if( pimsid == NULL ) {
    return -1;
  }

  switch( num_vaults )
  {
    case 32:
      *pimsid = (uint32_t) ((addr >> shiftamt) & 0x1F);
      break;
    case 16:
      *pimsid = (uint32_t) ((addr >> shiftamt) & 0xF);
      break;
    default:
      return -1;
      break;
  }
  return 0;
}

/* EOF */

/* --------------------------------------------- GETSHIFTAMOUNT */

extern int getshiftamount( uint32_t bsize,
                    			 uint32_t *shiftamt )
{
  switch( bsize )
  {
    case 32 :
      *shiftamt = 5;
      break;
    case 64 :
      *shiftamt = 6;
      break;
    case 128:
     *shiftamt = 7;
      break;
    case 256:
      *shiftamt = 8;
      break;
    default:
      return -1;
      break;
  }
	return 0;
}

/* EOF */

/* --------------------------------------------- CASH SIMULATION */

/* This function is used when performing a plru cache simulation. It generates *
 * a binary tree with n nodes where n = number_nodes. The nodes are inserted   *
 * one at a time in such a way that the posiion attriubute corresponds to a    *
 * level order traversal. A pointer to the tree is returned                    */
extern tree_node* create_tree(unsigned number_nodes)
{
  /* Create root node with position = 0*/
  tree_node *tree_root = create_tree_node(0,0);

#ifdef BSTDEBUG
  printf("\nInserting node %d as root node.\n", tree_root->position);
#endif

  /* Loop for i = 1 to i<number of nodes creating and inserting nodes at position = i */
  for(unsigned i = 1; i<number_nodes; i++)
  {
    queue *current_queue = create_queue(); /* create new queue structure to traverse to first unused place in tree*/
    enqueue(current_queue, tree_root); /* Enqueue root node*/

    tree_node *temp = create_tree_node(i, 0); /* New node to be placed in tree*/
    tree_node *cursor; /* cursor to traverse tree in level order*/

    /* continue traversing until the end of tree*/
    while(current_queue->front!=NULL)
    {
      /* pop from queue*/
      cursor = dequeue(current_queue);
      /* if at the end of tree and left child == null insert node*/
      if(cursor->left_child==NULL)
      {
        cursor->left_child = temp;
#ifdef BSTDEBUG
        printf("\nInserting node %d left of node %d.\n", temp->position, cursor->position);
#endif
        break;
      }
      /* otherwise enqueue left child */
      else
      {
        enqueue(current_queue, cursor->left_child);
      }
      /* if at the end of tree, left child != null, right child ==null, insert node*/
      if(cursor->right_child==NULL)
      {
#ifdef BSTDEBUG
        printf("\nInserting node %d right of node %d.\n", temp->position, cursor->position);
#endif
        cursor->right_child = temp;
        break;
      }
      /* otherwise enqueue right child */
      else
      {
        enqueue(current_queue, cursor->right_child);
      }
    }
  }
  /* return pointer to root with nodes inserted */
  return tree_root;
}

/* Function to create a bst node used in the plru calculation */
extern tree_node* create_tree_node(unsigned position, unsigned flag)
{
  tree_node *temp = malloc(sizeof(tree_node));
  temp->position = position;
  temp->flag = flag;
  temp->left_child = NULL;
  temp->right_child = NULL;
  return temp;
}

/* Enqueue a provided tree_node into the specified queue */
void enqueue(queue* queue, tree_node* node)
{
  // if queue is empty
  if(queue->rear==NULL)
  {
    queue->rear = node;
    queue->front = node;
  }
  else
  {
    queue->rear->next = node;
    queue->rear = node;
  }
}

/* Dequeue the first tree_node in the specified queue and return a pointer to it */
tree_node* dequeue(queue* queue)
{
  //if queue is empty
  if(queue->front == NULL)
  {
    return NULL;
  }
  else
  {
    //pop and set front to new front
    tree_node *temp = queue->front;
    queue->front = queue->front->next;
    if(queue->front == NULL)
    {
      queue->rear = NULL;
    }
    return temp;
  }
}

/* This function is called when a hit occurs during the simulation of a plru  *
 * n-way associative cache. It updates the flags along the path to the        *
 * specified way to direct a plru search away from that way. The flags are    *
 * are set accoring to the bitwise complement of the desired way at each level*/
extern void hit_update_bst(tree_node* root, unsigned total_ways, unsigned target_way)
{
  #ifdef BSTDEBUG
    printf("\nHit occured! Updating BST Tree.\n");
  #endif

  tree_node *cursor = root; // pointer to root of bst tree

  unsigned n = log2(total_ways); //number of bits needed = number of loop iterations

  unsigned m = n-1; //counter for number of bits to shift off

 /* loop for number of bits necessary, isolating the most significan bit first and then moving towards the least signicant bit */
  for(int i=0; i<n; i++)
  {
    unsigned flag_bit = (((target_way>>m)%2)==0) ? 1 : 0; //convoluted way to move across number's binary form and isolate one digit at a time, then store the complement
    cursor->flag = flag_bit; // Set flag to value

    /* Debugging statements */
    #ifdef BSTDEBUG
      printf("\nTarget Way: %u, Target Way >> m: %u, *%%2: %u, *~: %u, flag bit: %u\n", target_way, (target_way>>m), ((target_way>>m)%2), (~((target_way>>m)%2)), flag_bit);
      printf("\nSetting flags for hit at target_way %u.\n", target_way);
      printf("Setting flag at position %u to value %u.\n", cursor->position, cursor->flag);
    #endif

    // flag bit one indicates go right for plru way. so go left on path to current way
    if(flag_bit==1)
    {
      cursor = cursor->left_child; //move cursor to left child

      #ifdef BSTDEBUG
        printf("Moving cursor to left_child\n");
      #endif
    }
    // flag bit one indicates go left for plru way. so go right on path to current way
    else if(flag_bit==0)
    {
      cursor = cursor->right_child; // move cursor to right child

      #ifdef BSTDEBUG
        printf("Moving cursor to right_child\n");
      #endif
    }
    m--; //decrement m so move to next most signifcant bit on next iteration
  }
}

/* This function is used during a plru n-way cache simulation to find which   *
 * way is targeted for replacement. This is done by reading the flag bit at   *
 * each level of the tree, calculating its contribution in binary form, and   *
 * summing these values. The resulting value is the value of the way least    *
 * recently used (according to the plru algorithm) and is returned.           */
extern unsigned find_plru_way(tree_node* root, unsigned total_ways)
{
  #ifdef BSTDEBUG
    printf("\nMiss Occured! Cache Full! Determining PLRU way option.\n");
  #endif

  tree_node *cursor = root; // Cursor for tree staring at root
  unsigned accumulator = 0; // Accumulated value to be returned
  unsigned two_power = (log2(total_ways)-1); //Power of two to muliply flag by is equal to log2(total_ways) - 1. Decreases by one at each level of the tree.

  //Continue while there is another node to read
  while(cursor != NULL)
  {
    unsigned flag_value = cursor->flag;
    accumulator = (accumulator + (flag_value*(1<<two_power))); // Accumulated value = value + (flag * (2^(two_power)))

    #ifdef BSTDEBUG
      printf("Flag value %u at position %u. Adding %u to value at depth %u.\n", flag_value, cursor->position, (flag_value*(1<<two_power)), two_power);
    #endif

    two_power--; //as depth increases two_power decreases

    /* if flag == 0 go to left child */
    if(flag_value==0)
    {
      cursor = cursor->left_child;

      #ifdef BSTDEBUG
        printf("Moving to left child.\n");
      #endif
    }
    /* if flag == 1 go to right child */
    else if(flag_value==1)
    {
      cursor = cursor->right_child;

      #ifdef BSTDEBUG
        printf("Moving to right child.\n");
      #endif
    }
  }

  #ifdef BSTDEBUG
    printf("Returning way value %u as replacment candidate.\n", accumulator);
  #endif
  //Value returned is equal to way that needs to be replaced
  return accumulator;
}
