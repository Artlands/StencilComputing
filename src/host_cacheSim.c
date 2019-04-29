/*
 * _HOST_CASHESIM_C_
 *
 * FUNCTION TO SIMULATE CACHE ON HOST SIDE
 * PLRU reused from Brody's CacheSimulator program
 */

/* Included files*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "pims.h"

// #define DEBUG

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int read_trace( FILE *infile, trace_node *trace);
extern void write_to_file(FILE* fp, char* op, int num_bytes, int procid, uint64_t addr);

tree_node* create_tree(unsigned number_nodes);
tree_node* create_tree_node(unsigned position, unsigned flag);
queue* create_queue();
void enqueue(queue* queue, tree_node* node);
tree_node* dequeue(queue* queue);
void hit_update_bst(tree_node* root, unsigned total_ways, unsigned target_way);
unsigned find_plru_way(tree_node* root, unsigned total_ways);

/*
 * Main Function, Takes command line arguments, read physical memory address
 * traces, simulate 'n-way' cache, generate memory access trace if cache miss
 *
 */
int main(int argc, char* argv[])
{
  /* vars */
  int i = 0;
  int j = 0;
  int ret = 0;
  int done = 0;

  /* MEMORY TRACE */
  char infilename[1024];
  char outfilename[1024];
  FILE *infile = NULL;
  FILE *outfile = NULL;
  trace_node trace;

  /* CACHE */
  CACHE current_cache;
  /* Init cache */
  current_cache.ways = WAYS;
  current_cache.sets = SETS;
  uint64_t tag_field[current_cache.sets][current_cache.ways];
  unsigned valid_field[current_cache.sets][current_cache.ways];
  tree_node *trees[current_cache.sets];

  /* For each address */
  uint64_t block_addr;
  uint64_t index;
  uint64_t tag;
  int processed_flag = 0;
  unsigned replacement;

  while( (ret = getopt( argc, argv, "f:h")) != -1 )
  {
    switch( ret )
    {
      case 'f':
        sprintf(infilename, "%s", optarg);
        break;
      case 'h':
        printf("%s%s%s\n", "usage : ", argv[0], " -fh");
        printf(" -f <Trace file name>\n");
        printf(" h ...print help\n");
        return 0;
        break;
      default:
        printf("%s%s%s\n", "Unknown option: see ", argv[0], " -fh" );
        return -1;
        break;
    }
  }
  /* ---- Sanity Check ----*/
  infile = fopen(infilename, "r");
  if( infile == NULL ){
    printf("ERROR: Cannot open input memory trace file\n");
    return -1;
  }

  sprintf(outfilename, "%s%s", infilename, "cached");

  outfile = fopen(outfilename, "w");
  if( outfile == NULL ){
    printf("ERROR: Cannot open output memory trace file\n");
    return -1;
  }
  /* ---- End Sanity Check ---- */

  /* Initialization of arrays*/
  for(int i=0; i<current_cache.sets; i++)
  {
    for(int j=0; j<current_cache.ways; j++)
    {
      valid_field[i][j] = 0;
      tag_field[i][j] = 0;
    }
  }

  /* Calls function to create BST trees for each set in the cache *
   * using n-1 nodes where n is the number of ways. Sets pointer  *
   * at appropriate index of tree array                           */
  for(int i=0; i<current_cache.sets; i++)
  {
    trees[i] = create_tree(current_cache.sets-1);
  }

  /* read first request from the input file */
  done = read_trace( infile, &trace );
  while( done != 0 ){
    done = read_trace( infile, &trace);
  }

  /* read all traces until to the end of file */
  while( done == 0 ){
    /* get address infor from input trace */
    block_addr = (uint64_t)( trace.addr >> (unsigned)log2(BLOCK_SIZE) );
    index = (uint64_t)( block_addr % current_cache.sets );
    tag = (uint64_t)( block_addr >> (unsigned)log2(current_cache.sets) );

#ifdef DEBUG
    printf("Memory address: %llu, Sets: %u, Ways: %u, Block address: %llu, Index: %llu, Tag: %llu ",
           trace.addr, current_cache->sets, current_cache->ways, block_addr, index, tag);
#endif
    /* Flag is set to 1 when address is processed */
    processed_flag = 0;

    /* check for hit in appropriate set */
    for( i = 0; i < current_cache.ways; i++ )
    {
      /* If cache space is occupied and tag matches then Hit*/
      if( (valid_field[index][i]) && (tag_field[index][i] == tag) )
      {
        current_cache.hits ++;
        hit_update_bst(trees[index], current_cache.ways, i);
        processed_flag = 1;
        break;
      }
    }
    /* If not a hit then process miss */
    if( processed_flag == 0 )
    {
      current_cache.misses ++;
      /* write this address to output trace file */
      write_to_file(outfile, trace.op, trace.num_bytes, trace.procid, trace.addr);

      /* First look for an unused way in the proper set */
      for( i = 0; i < current_cache.ways; i++ )
      {
        if( valid_field[index][i] == 0 )
        {
          valid_field[index][i] = 1;
          tag_field[index][i] = tag;
          processed_flag = 1;
          break;
        }
      }
      /* If all ways are occupied then traverse bst and replace indicated way*/
      if( processed_flag == 0 )
      {
        replacement = find_plru_way(trees[index], current_cache.ways);
        tag_field[index][replacement] = tag;
      }
    }
    /* read next request from the input file until to the end of file*/
    done = read_trace( infile, &trace );
    while( done != 0 ){
      done = read_trace( infile, &trace);
      if( done == -2) break;
    }
  }
  current_cache.hit_rate = (((double)(current_cache.hits))/((double)(current_cache.hits+current_cache.misses)));
  current_cache.miss_rate = (((double)(current_cache.misses))/((double)(current_cache.hits+current_cache.misses)));

  printf("Cache hit rate: %f\n", current_cache.hit_rate);
  printf("Cache miss rate: %f\n", current_cache.miss_rate);

  fclose(infile);
  fclose(outfile);

  return 0;
}

/* This function is used when performing a plru cache simulation. It generates *
 * a binary tree with n nodes where n = number_nodes. The nodes are inserted   *
 * one at a time in such a way that the posiion attriubute corresponds to a    *
 * level order traversal. A pointer to the tree is returned                    */
tree_node* create_tree(unsigned number_nodes)
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
tree_node* create_tree_node(unsigned position, unsigned flag)
{
  tree_node *temp = malloc(sizeof(tree_node));
  temp->position = position;
  temp->flag = flag;
  temp->left_child = NULL;
  temp->right_child = NULL;
  return temp;
}

queue* create_queue()
{
  queue* temp = (queue*)malloc(sizeof(queue));
  temp->front = NULL;
  temp->rear = NULL;
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
void hit_update_bst(tree_node* root, unsigned total_ways, unsigned target_way)
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
unsigned find_plru_way(tree_node* root, unsigned total_ways)
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
