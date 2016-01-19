/// File: table.c
///	Author: pxz5572, Paul Zenie
/// 
/// A basic hashtable
///

#include <stdlib.h>
#include <assert.h>
#include "hash.h"
#include "table.h"

/// Create a new hash table.
/// @param hash The key's hash function
/// @param equals The key's equal function for comparison
/// @param print A print function for the key/value, used for dump debugging
/// @exception Assert fails if can't allocate space
/// @return A newly created table
Table* create( long (*hash)(void* key), bool (*equals)(void* key1, void* key2),
				void (*print)(void* key1, void* key2))
{
	Table* tab = malloc(sizeof(Table));
	if(tab == NULL)
	{
		fprintf(stderr, "Space not allocated.");
		assert(NULL);
	}
	tab->hash = hash;
	tab->equals = equals;
	tab->print = print;
	tab->size = 0;
	tab->capacity = INITIAL_CAPACITY;
	tab->collisions = 0;
	tab->rehashes = 0;
	tab->table = (Entry**) calloc(INITIAL_CAPACITY, sizeof(Entry*));
	if(tab->table == NULL)
	{	
		fprintf(stderr, "Space not allocated");
		assert(NULL);
	}
	return tab;
}

/// Destroy a table
/// @param t The table to destroy
void destroy( Table* tab ){
	for(size_t i = 0; i < tab->capacity; i++)
	{
		Entry* ent = tab->table[i];
		if(ent != NULL)
		{
			free(ent);
		}
	}
	free(tab->table);
	free(tab);   
}

/// Print out information about the hash table (size,
/// capacity, collisions, rehashes).  If full is
/// true, it will also print out the entire contents of the hash table,
/// using the registered print function with each non-null entry.
/// @param t The table to display
/// @param full Do a full dump of entire table contents
void dump( Table* tab, bool full)
{
	printf("Collisions: %zu\n",tab->collisions);
	printf("Rehashes: %zu\n",tab->rehashes);
	printf("Size: %zu\n",tab->size);
	printf("Capacity: %zu\n",tab->capacity);
	if(!full) 
	{
		return;
	}
	for(size_t i = 0; i < tab->capacity; i++)
	{
		printf("%zu: ",i);
		if(tab->table[i] == NULL)
		{
			printf("null\n");
		}
		else
		{
			printf("(");
			tab->print((tab->table[i])->key, (tab->table[i])->value);
			printf(")\n");
		}	   
	}
}


/// Get the value associated with a key from the table.  This function
/// uses the registered hash function to locate the key, and the
/// registered equals function to check for equality.
/// @pre The table must have the key, or else it will assert fail
/// @param t The table
/// @param key The key
/// @return The associated value of the key
void* get( Table* tab, void* key )
{
	long hash = tab->hash(key);
	int index = hash % tab->capacity;
	Entry* curr = tab->table[index];
	while(curr != NULL)
	{
		if( tab->equals(curr->key,key) ) 
		{
			break;
		}
		tab->collisions++;
		if(index < (int)(tab->capacity - 1))
		index++;
		else
		{
			index = 0;
		}
		curr = tab->table[index];
	}
	if(curr == NULL)
	{
		assert(NULL);
	}
	return curr->value;
}

/// Check if the table has a key.  This function uses the registered hash
/// function to locate the key, and the registered equals function to
/// check for equality.
/// @param t The table
/// @param key The key
/// @return Whether the key exists in the table.
bool has( Table* tab, void* key )
{
	long hash = tab->hash(key);
	int index = hash % tab->capacity;
	Entry* curr = tab->table[index];
	while(curr != NULL)
	{
		if(tab->equals(curr->key,key))
		{
			return true;
		}
		tab->collisions++;
		if(index < (int)(tab->capacity - 1))
		{
			index++;
		}
		else
		{
			index = 0;
		}
		curr = tab->table[index];
	}
	return false;
}

/// Get the collection of keys from the table.  This function allocates
/// space to store the keys, which the caller is responsible for freeing.
/// @param t The table
/// @exception Assert fails if can't allocate space
/// @return A dynamic array of keys
void** keys( Table* tab )
{
	void** keys = (void**) calloc(tab->size, sizeof(void*));
	if(keys == NULL)
	{
		fprintf(stderr,"Space not allocated.");
		assert(NULL);
	}
	int added = 0;
	for(size_t i = 0; i < tab->capacity; i++)
	{
		if(tab->table[i] != NULL)
		{
			keys[added] = (tab->table[i])->key;
			added++;
		}
	}
	return keys;
}

///Rehash table to increase capacity.
///@param tab - Pointer to the table.
static void rehash( Table* tab )
{
	Entry** oldTab = tab->table;
	size_t oldCap = tab->capacity;
	size_t newCap = (tab->capacity * RESIZE_FACTOR);
	tab->capacity = newCap;
	tab->table = (Entry**) calloc( newCap, sizeof(Entry*) );
	if(tab->table == NULL)
	{
		fprintf(stderr, "Space not allocated.");
		assert(NULL);
	}
	tab->size = 0;
	for( size_t i = 0; i < oldCap; i++ )
	{
		if( oldTab[i] != NULL )
		{
			put(tab, oldTab[i]->key, oldTab[i]->value);
			free(oldTab[i]);
		}
	}
	free(oldTab);
	tab->rehashes++;
}

/// Add a key value pair to the table, or update an existing key's value.
/// This function uses the registered hash function to locate the key,
/// and the registered equals function to check for equality.
/// @param t The table
/// @param key The key
/// @param value The value
/// @exception Assert fails if can't allocate space
/// @return The old value in the table, if one exists.
void* put( Table* tab, void* key, void* value)
{
	long hash = tab->hash(key);
	int index = hash % tab->capacity;
	int found = 0;
	Entry* curr = tab->table[index];
	while(curr != NULL)
	{
		if(tab->equals(curr->key, key))
		{
			found = 1;
			break;
		}
		tab->collisions++;
		if(index < (int)(tab->capacity - 1))
		{
			index++;
		}
		else
		{
			index = 0;
		}
		curr = tab->table[index];
	}
	if(found == 1)
	{
		void* oldVal = curr->value;
		curr->value = value;
		return oldVal;
	}
	else
	{
		tab->table[index] = (Entry*) malloc(sizeof(Entry));
		if(tab->table[index] == NULL)
		{
			fprintf(stderr, "Space not allocated.");
			assert(NULL);
		}
		(tab->table[index])->key = key;
		(tab->table[index])->value = value;
		tab->size++;
		if(((double)tab->size / (double)tab->capacity) >= LOAD_THRESHOLD)
		{
			rehash(tab);
		}
	}
	return NULL;
}

/// Get the collection of values from the table.  This function allocates
/// space to store the values, which the caller is responsible for freeing.
/// @param t The table
/// @exception Assert fails if can't allocate space
/// @return A dynamic array of values
void** values( Table* tab )
{
	void** values = (void**) calloc( tab->size, sizeof(void*));
	if(values == NULL)
	{
		fprintf(stderr,"Space not allocated.");
		assert(NULL);
	}
	int added = 0;
	for(size_t i = 0; i < tab->capacity; i++)
	{
		if(tab->table[i] != NULL)
		{
			values[added] = (tab->table[i])->value;
			added++;
		}
	}
	return values;
}
