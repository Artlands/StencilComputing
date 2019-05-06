/*
 * _PIMS_H_
 *
 * PIMS HEADER
 *
 */

/* Physical Memory Parameters */
#define PAGESIZE 4096
#define SLOTS_PER_GB 1073741824

/* Masks for Virtual address */
#define VIRTUAL_PAGE_MASK 0xFFFFFFFFFFFFF000
#define VIRTUAL_OFFSET_MASK 0x0000000000000FFF
#define VIRTUAL_PAGE_SHIFT 12

/* Cache Parameters */
#define BLOCKSIZE 64
#define WAYS 8

/* ---------------------------------------------- DATA STRUCTURE*/

typedef struct pta_node
{
  int64_t virtual_page;
  int64_t page_frame;
  int64_t age;
}pta_node;

typedef struct cache_node
{
  int cache_size;
  int block_size;
  int ways;
  int sets;
  uint64_t dirty_eviction;
  uint64_t hits;
  uint64_t misses;
}cache_node;

typedef struct trace_node
{
 char op[10];
 int num_bytes;
 int procid;
 uint64_t addr;
}trace_node;

/* Node struct used in construction of BST for plru algorithm implementation */
typedef struct tree_node
{
 unsigned position; // Position of node in level order traversal of BST, only used for ease of debugging
 unsigned flag; // Flag value of a particular node
 struct tree_node *next; // only used for queue
 struct tree_node *left_child; // pointer to left child
 struct tree_node *right_child; // pointer to right child
}tree_node;

/* Queue used in construction of BST. Not really necessary for plru algorithm. Only *
 * needed to order the tree in the way I wanted to for easy debugging.              */
typedef struct queue
{
  struct tree_node *front;
  struct tree_node *rear;
}queue;

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
