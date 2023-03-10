#include "../include/list.h"
#include "../include/mythread.h"
#define SZ 4096
#include <stdlib.h>
#include <string.h>

struct hashmap_element_s {
  const char *key; // I have changed this!
  void *data;
};

struct hashmap_s {
  struct list* table[SZ];
  struct lock* lk[SZ];
};

int hash_func(const char* key){ // Hash function for the hash map
    int i = 0;
    int sum = 0;
    int prod = 0;
    
    while (*(key + i) != '\0'){
      if (i % 4 == 0){
        sum += prod;
        prod = *(key + i);
      }else{
        prod = prod*100 + *(key + i);
      }
      i++;
    }
    sum += prod;
    sum = sum % SZ;
    if (sum < 0) return sum + SZ;
    return sum;
    
}

int hashmap_create(struct hashmap_s *const out_hashmap){ // Initialize a hashmap
  
  for (int i = 0; i < SZ; i++){ // For i from 0 to 4095 (size of table)
    *((out_hashmap -> table) + i) = list_new() ; // ith index of the table contains pointer to an empty list now
    out_hashmap->lk[i] = lock_new();
  }
  
  return 0; // No reason for this to fail, hence. no return -1
}   

int hashmap_put(struct hashmap_s *const hashmap, const char* key, void* data){ // Set value of the key as data in hashmap. You can use any method to resolve conflicts. Also write your own hashing function
  char* mapKey = malloc(sizeof(key));

  for (int i=0; i < sizeof(mapKey)/sizeof(char); i++) {
    mapKey[i] = key[i];
  }

  int hash_value = hash_func(mapKey); // Calculating the hash value for the given key

  struct list* l = *((hashmap -> table) + hash_value); // Pointer to the linked list at the corresponding hash value in the table
  struct listentry *p = l -> head; // Pointer to the head of that list
  
  while(p){ // While p is not NULL
    struct hashmap_element_s* x = p -> data; // Pointer to the hashmap element at p
    if (strcmp(x -> key, mapKey) == 0){ // If key of the hashmap element matches the argument key
      x -> data = data; // Update the data for the key
      return 0; // Successful update
    }
    p = p -> next; // p goes to p -> next
  }
  
  // Code comes here when the key doesn't already exist
  struct hashmap_element_s *t = malloc(sizeof(struct hashmap_element_s)); // Declaring the memory for the hashmap element
  t -> key = mapKey; // t -> key is set as key
  t -> data = data; // t -> data is set as data
    
  list_add(*((hashmap -> table) + hash_value), t); // Append the hashmap element to the corresponding hash value list
  
  return 0; // Successful add
}
  
void* hashmap_get(struct hashmap_s *const hashmap, const char* key){ // Fetch value of a key from hashmap
  
  int hash_value = hash_func(key); // Finding out the hash value
  
  struct list* l = *((hashmap -> table) + hash_value); // Pointer to the list corresponding to the hash value
  struct listentry *p = l -> head; // Pointer to the head of the list
  
  while(p){ // While p is not NULL
    struct hashmap_element_s* t = p -> data; // Pointer to the hash element pointed by p -> data
    if (strcmp(t -> key, key) == 0) return (t -> data); // If the strings are the same then return the data in that hash element
    p = p -> next; // p is set to p -> next
  }
  return NULL; // Return NULL if the key is not found
  
} 

void hashmap_iterator(struct hashmap_s* const hashmap, int (*f)(struct hashmap_element_s *const)){ // Execute argument function on each key-value pair in hashmap
  
  for (int i = 0; i < SZ; i++){
    struct list* l = *((hashmap -> table) + i);
    struct listentry* p = l -> head;
    
    while(p){
      struct hashmap_element_s* t = p -> data;
      f(t);
      p = p -> next;
    }
  }
}
int acquire_bucket(struct hashmap_s *const hashmap, const char* key){   // Acquire lock on a hashmap slot
  int hash_value = hash_func(key);

  lock_acquire(hashmap->lk[hash_value]);
}
int release_bucket(struct  hashmap_s *const hashmap, const char* key){   // Release acquired lock
  int hash_value = hash_func(key);

  return lock_release(hashmap->lk[hash_value]); 
}