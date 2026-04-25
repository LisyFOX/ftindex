#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#include "hash_table.h"


// Create hash table function
struct hash_table_t * create_hash_table(int bucket_num, hash_function_t hash, compare_function_t compare) {
    // Allocate memory for hash table and check if was successfully allocated
    struct hash_table_t *hash_table = malloc(sizeof(struct hash_table_t));
    if (hash_table == NULL) {
        return NULL;
    }

    // Allocate memory for buckets and check if was successfully allocated
    hash_table->buckets = calloc(bucket_num, sizeof(struct node_t*));
    if (hash_table->buckets == NULL) {
        return NULL;
    }

    // Allocate memory for bucket locks and initialize mutexes
    hash_table->locks = malloc(bucket_num * sizeof(pthread_mutex_t));
    if (hash_table->locks == NULL) {
        return NULL;
    }

    for (int i = 0; i < bucket_num; i++)
    {
        if (pthread_mutex_init(&hash_table->locks[i], NULL) != 0) { 
            printf("Unable to initialize mutex!");
            return NULL;
        } 
    }



    hash_table->bucket_num = bucket_num;
    hash_table->hash = hash;
    hash_table->compare = compare;

    return hash_table;
}

// Create node function
struct node_t * create_node(void *key, void *value, struct node_t *next) {
    // Allocate memory for node and check if it was successfully allocated
    struct node_t *node = malloc(sizeof(struct node_t));
    if (node == NULL) {
        return NULL;
    }

    // Fill node values with arguments
    node->key = key;
    node->value = value;
    node->next = next;

    return node;
}

struct node_t * get_node(struct hash_table_t * hash_table, int bucket_num) {
    return hash_table->buckets[bucket_num];
}

// Insert node to hash table function
void hash_table_insert(struct hash_table_t *hash_table, int bucket_num, void *key, void *value) {
    // Acquire the number of bucket where the key goes in
    int bucket_index = hash_table->hash(key) % bucket_num;

    // Lock this bucket
    pthread_mutex_lock(&hash_table->locks[bucket_index]);
    
    // Create a node and say it doesn't exist in a hash table yet
    struct node_t *node = create_node(key, value, NULL);
    int exists_in_hash_table = 0;

    // Fill the bucket with the node
    if (hash_table->buckets[bucket_index] == NULL) {
        hash_table->buckets[bucket_index] = node;
    } else {
        struct node_t *current_node = hash_table->buckets[bucket_index];

        // Loop through every node inside the bucket
        while (current_node != NULL)
        {
            // Say a node with the same key and value already exists and exit the loop
            if (hash_table->compare(key, current_node->key) == 0 &&
                hash_table->compare(value, current_node->value) == 0) {
                exists_in_hash_table = 1;
                break;
            }

            // Insert the node into the end of the list and unlock the bucket 
            if (current_node->next == NULL) {
                current_node->next = node;
                pthread_mutex_unlock(&hash_table->locks[bucket_index]);
                return;
            }

            // Move further in loop
            current_node = current_node->next;
        }

        // Unlock the bucket if a node already exists
        if (exists_in_hash_table) {
            pthread_mutex_unlock(&hash_table->locks[bucket_index]);
            return;
        }
    }

    // Unlock this bucket
    pthread_mutex_unlock(&hash_table->locks[bucket_index]);
}

void destroy_hash_table(struct hash_table_t *hash_table)
{
    for (int i = 0; i < hash_table->bucket_num; i++) {
        pthread_mutex_destroy(&hash_table->locks[i]);
    }

    free(hash_table->locks);
    free(hash_table->buckets);
    free(hash_table);
}