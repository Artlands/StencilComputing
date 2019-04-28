/*
 * _PIMS_H_
 *
 * PIMS HEADER
 *
 */

/* Physical Memory Parameters */
#define PAGESIZE 4096
#define GB 1073741824

/* Cache Parameters */
// #define CACHE_SIZE 32768      // Total cache size 32KB
// #define CACHE_SIZE 131072     // Total cache size 128KB
#define CACHE_SIZE 8388608       // Total cache size 8MB
// #define BLOCK_SIZE 128
#define BLOCK_SIZE 256
#define NUM_BLOCKS (CACHE_SIZE/BLOCK_SIZE)
#define WAYS 8
#define SETS (NUM_BLOCKS/WAY)

/* Masks for Virtual address */
#define VIRTUAL_PAGE_MASK 0xFFFFFFFFFFFFF000
#define VIRTUAL_OFFSET_MASK 0x0000000000000FFF
#define VIRTUAL_PAGE_SHIFT 12

/* ---------------------------------------------- DATA STRUCTURE*/

typedef struct pta_node
{
  int64_t virtual_page;
  int64_t age;
}pta_node;

typedef struct trace_node
{
 char op[10];
 int num_bytes;
 int procid;
 uint64_t addr;
}trace_node;

typedef struct cache_node
{
  uint64_t sets;
  uint64_t ways;                 // ways per set
  uint64_t hits;
  uint64_t misses;
  double hit_rate;
  double miss_rate;
} cache_node;

/* Node struct used in construction of BST for plru algorithm implementation */
typedef struct tree_node
{
 unsigned position; // Position of node in level order traversal of BST, only used for ease of debugging
 unsigned flag; // Flag value of a particular node
 struct tree_node *next; // only used for queue
 struct tree_node *left_child; // pointer to left child
 struct tree_node *right_child; // pointer to right child
}tree_node;

/* ---------------------------------------------- FUNCTION PROTOTYPES*/
extern int read_trace( FILE *infile, trace_node *trace);

//
// /*
//  * struct pimsbuf
//  * contains struct data for pims buffer
//  */
//
// struct pimsbuf{
//   uint32_t pimsid;          // pims id
//   uint32_t in_size;         // input buffer size
//   uint32_t out_size;        // output buffer size
//   uint64_t *in_addr;        // save input address array
//   struct flit *out_addr;    // save output FLICT array
// };
//
// struct flit{
//   uint32_t flitid;
//   int size;
//   uint64_t addr[16];
// };
