/*
 * Brody Williams
 * Cache Simulator
 * CS 3375 - Fall 2017 - Texas Tech University
 * CacheSimulator.h - Header file
 * Some sections and/or pieces of code reused from Dr. Chen's cachesim program
 */

 /* Preprocessor defintions - Values supplied before compilation */
 #define CACHE_SIZE 32768 // Total cache size (in bytes)
 #define BLOCK_SIZE 128 // Size of an individual black (in bytes)
 #define NUM_BLOCKS (CACHE_SIZE/BLOCK_SIZE)
 #define OUTPUT_FILE "TestingFile.txt" // Name of file where results are written

 /* Used for conditional compilation of debugging sections */
 //#define DEBUG // Debugging sections for sections not related to plru algorithm or BST construction
 //#define BSTDEBUG // Debugging sections related specifically to plru algorithm or BST construction

/* Generic cache struct common to all types of caches. */
 typedef struct
 {
   char cache_type[27]; //Type of cache - direct, fully-associative, or n-way
   char replacement_algorithm[4]; //Replacement algorithm used for cache when full
   unsigned sets;
   unsigned ways; // ways per set
   unsigned hits;
   unsigned misses;
   double hit_rate;
   double miss_rate;
 }Cache;

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


 /* Function Prototypes */
 uint64_t convert_address(char memory_addr[]);

 void run_dmc_simulation(Cache* cache, char* trace_file_name);

 void run_fac_simulation(Cache* cache, char* trace_file_name);

 void run_nru_simulation(Cache* cache, char* trace_file_name);

 void run_plru_simulation(Cache* cache, char* trace_file_name);

 void run_rr_simulation(Cache* cache, char* trace_file_name);

 void print_results(Cache* cache, char* trace_file_name);

 void write_to_file(Cache* cache, char* trace_file_name);

 tree_node* create_tree(unsigned number_nodes);

 queue* create_queue();

 void enqueue(queue* queue, tree_node *node);

 tree_node* dequeue(queue* queue);

 tree_node* create_tree_node(unsigned position, unsigned flag);

 void hit_update_bst(tree_node* root, unsigned total_ways, unsigned target_way);

 unsigned find_plru_way(tree_node* root, unsigned total_ways);
