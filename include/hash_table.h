#include <pthread.h>

typedef int (*hash_function_t)(void *key);
typedef int (*compare_function_t)(void *a, void *b);

struct hash_table_t
{
    struct node_t **buckets;
    int bucket_num;

    pthread_mutex_t *locks;

    hash_function_t hash;
    compare_function_t compare;
};


struct node_t
{
    void *key;
    void *value;
    struct node_t *next;
};


struct hash_table_t * create_hash_table(int bucket_num, hash_function_t hash, compare_function_t compare);
void hash_table_insert(struct hash_table_t *hash_table, int bucket_num, void *key, void *value);

struct node_t * create_node(void *key, void *value, struct node_t *next);
struct node_t * get_node(struct hash_table_t * hash_table, int bucket_num);
