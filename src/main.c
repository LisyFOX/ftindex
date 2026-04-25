#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#include "hash_table.h"

#define MAX_WORD_LENGTH 1024

struct thread_args_t
{
    struct hash_table_t * hash_table;
    int bucket_num;
    char* file_name;
};


static void *thread_func(struct thread_args_t *arg)
{
    char word_buffer[MAX_WORD_LENGTH];
    char *file_name = arg->file_name;

    // Open input file
    int fd = open(file_name, O_RDONLY);

    // Get input file size
    struct stat st;
    fstat(fd, &st);
    int length = st.st_size; // velikost dat

    // Loop variables
    char *data;
    int i = 0;
    int offset = 0;

    // Map that data block to memory and save mapped address
    data = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0);
    void* mmap_addr = data;

    while (offset <= length)
    {
        // Print a word or pass a character to buffer
        if ((
            !(*data >= 'A' && *data <= 'Z') &&
            !(*data >= 'a' && *data <= 'z')
            ) ||
            (i >= MAX_WORD_LENGTH - 1)
        ) {
            word_buffer[i] = '\0';

            if (i > 0) {
                char *word = malloc(strlen(word_buffer) + 1);
                strcpy(word, word_buffer);

                hash_table_insert(arg->hash_table, arg->bucket_num, word, file_name);
            }

            i = 0;
        } else {
            word_buffer[i] = *data;
            i++;
        }

        // Move further if we haven't stuck on the max word length
        if (i < MAX_WORD_LENGTH - 1) {
            data++;
            offset++;
        }
    }

    // Unmap input file
    munmap(mmap_addr, length);

    // Close input file
    close(fd);

    return 0;
}


// Hash word function
int hash_word(void *key) {
    char *word = key;
    int hash = 0;

    // Sum its ASCII character values to obtain the hash
    while (*word != '\0')
    {
        hash = hash + *word;
        word++;
    }

    return hash;
}

// Compare words function
int compare_words(void *a, void *b) {
    return strcmp(a, b);
}

int main(int argc, char *argv[]) {
    // Check if only 3 arguments were given 
    if (argc < 3) {
        printf("Bad argument amount given!");
        exit(1);
    }

    // Parse amount of buckets and check if it was given correctly
    int bucket_num = atoi(argv[1]);
    if (bucket_num <= 0) {
        printf("Unable to get buckets number argument!");
        exit(1);
    }

    // Get file count and create files array variable
    int file_count = argc - 2;
    char** files;

    // Allocate memory for files array
    files = malloc(file_count * sizeof(char*));

    // Fill files array with the arguments given
    for (int i = 0; i < file_count; i++)
    {
        // Allocate memory for file name in array and fill it with the argument given
        files[i] = malloc((strlen(argv[i + 2]) + 1) * sizeof(char));
        strcpy(files[i], argv[i + 2]);
    }

    
    // Create hash table and check if it was created successfully
    struct hash_table_t *hash_table = create_hash_table(bucket_num, hash_word, compare_words);
    if (hash_table == NULL) {
        printf("Unable to create a hash table");
        exit(1);
    }        
    

    pthread_t thread[file_count];

    // Create threads
    for (int i = 0; i < file_count; i++)
    {
        // Allocate memory for arg structure and fill its values with the arguments given
        struct thread_args_t *args = malloc(sizeof(struct thread_args_t));
        args->hash_table = hash_table;
        args->bucket_num = bucket_num;
        args->file_name = files[i];

        // Create a thread and check if it was successfully created
        if (pthread_create(&thread[i], NULL, (void*) thread_func, args)) {
            fprintf(stderr, "Unable to create a thread\n");
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < file_count; i++)
    {   
        pthread_join(thread[i], NULL);
    }

    // Read hash table
    for (int i = 0; i < bucket_num; i++)
    {
        // Assign the current node to the beginning of the bucket
        struct node_t *current_node = get_node(hash_table, i);

        // Allocate memory for traversed words and initialize traversed words count
        int traversed_words_count = 0;
        char **traversed_words = malloc(sizeof(char*));

        // Loop through the bucket
        while (current_node != NULL) {
            int is_in_traversed_words = 0;

            char *current_word = current_node->key;

            // Check if this node's word is in traversed words and if so, exit the loop
            for (int i = 0; i < traversed_words_count; i++)
            {
                if (strcmp(current_word, traversed_words[i]) == 0) {
                    is_in_traversed_words = 1;
                    break;
                }
            }

            // Skip node if this word was in traversed words
            if (is_in_traversed_words) {
                current_node = current_node->next;
                continue;
            }

            // Reallocate traversed words memory and append current word
            char** reallocated_traversed_words = realloc(traversed_words, (sizeof(char*) * traversed_words_count + 1));
            traversed_words = reallocated_traversed_words;

            traversed_words[traversed_words_count] = current_word;

            // Print current word
            printf("%s:", current_word);
            
            // Use a traverse node pointer to traverse remaining hash table's bucket to print all file names related to current word
            struct node_t *traverse_node = current_node;

            while (traverse_node != NULL) {
                char *traverse_word = traverse_node->key;
                char *traverse_file_name = traverse_node->value;

                // Print a file name if traverse node has current word
                if (strcmp(current_word, traverse_word) == 0) {
                    printf(" %s", traverse_file_name);
                }

                traverse_node = traverse_node->next;
            }
            
            printf("\n");

            // Move futher in loop and increment traversed words count
            current_node = current_node->next;
            traversed_words_count++;
        }

        // Free allocated memory for every word in traversed words
        for (int i = 0; i < traversed_words_count; i++)
        {
            free(traversed_words[i]);
        }

        // Free allocated memory for traversed words array
        free(traversed_words);
    }

    // Free allocated memory for every file name in files array
    for (int i = 0; i < file_count; i++)
    {
        free(files[i]);
    }
    
    // Free the array of files
    free(files);

    // Free allocated memory for hash table
    destroy_hash_table(hash_table);

    return 0;
}