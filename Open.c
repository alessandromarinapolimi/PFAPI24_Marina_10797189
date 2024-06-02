#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define WORD_SIZE 256    // Size of a single word
#define INITIAL_SIZE 768 // Initial size of the line buffer (WORD_SIZE * 3)
#define TABLE_SIZE 1024  // Size of the hash table

unsigned int courier_frequency, courier_capacity, num_recipes = 0, max_recipes = 0, num_ingredients_total = 0, max_ingredients_total = 0, time = 0;

// Structure to represent a batch of an ingredient
typedef struct
{
    unsigned int quantity;
    unsigned int expiration_time;
} Batch;

// Structure to represent an ingredient
typedef struct
{
    char name[WORD_SIZE];
    unsigned int total_quantity;
    Batch *batches;
    unsigned int num_batches;
    unsigned int max_batches;
} Ingredient;

Ingredient *ingredients_total = NULL; // Dynamic array of total ingredients

// Structure to represent a recipe
typedef struct
{
    char name[WORD_SIZE];
    Ingredient *ingredients;
    unsigned int num_ingredients;
} Recipe;

Recipe *recipes = NULL; // Dynamic array of recipes

// Node structure for the hash table
typedef struct HashNode
{
    char *key;
    unsigned int value;
    struct HashNode *next;
} HashNode;

// Hash table structure
typedef struct
{
    HashNode **table;
} HashTable;

// Function to create a new hash table
HashTable *create_table()
{
    HashTable *new_table = malloc(sizeof(HashTable));
    new_table->table = malloc(TABLE_SIZE * sizeof(HashNode *));
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        new_table->table[i] = NULL;
    }
    return new_table;
}

// Hash function
unsigned int hash_function(char *key)
{
    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    for (; i < key_len; i++)
    {
        value = value * 37 + key[i];
    }
    value = value % TABLE_SIZE;

    return value;
}

// Function to insert into the hash table
void hash_table_insert(HashTable *hashtable, char *key, unsigned int value)
{
    unsigned int index = hash_function(key);
    HashNode *new_node = malloc(sizeof(HashNode));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = hashtable->table[index];
    hashtable->table[index] = new_node;
}

// Function to find an index in the hash table
int hash_table_find(HashTable *hashtable, char *key)
{
    unsigned int index = hash_function(key);
    HashNode *node = hashtable->table[index];
    while (node != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            return node->value;
        }
        node = node->next;
    }
    return -1;
}

// Function to free the hash table
void free_table(HashTable *hashtable)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        HashNode *node = hashtable->table[i];
        while (node != NULL)
        {
            HashNode *temp = node;
            node = node->next;
            free(temp->key);
            free(temp);
        }
    }
    free(hashtable->table);
    free(hashtable);
}

// Hash tables for ingredients and recipes
HashTable *ingredient_table;
HashTable *recipe_table;

// Function to find the index of a recipe by name
unsigned int find_recipe_index(char *name)
{
    return hash_table_find(recipe_table, name);
}

// Function to find the index of an ingredient by name
unsigned int find_ingredient_index(char *name)
{
    return hash_table_find(ingredient_table, name);
}

// Function to initialize a new recipe
void init_recipe(Recipe *recipe, char *name, unsigned int num_ingredients)
{
    strcpy(recipe->name, name);
    recipe->ingredients = malloc(num_ingredients * sizeof(Ingredient));
    recipe->num_ingredients = num_ingredients;
}

// Function to compare batches by expiration time using binary insertion
unsigned int find_insert_index_binary(Batch *batches, unsigned int num_batches, unsigned int expiration_time)
{
    unsigned int low = 0;
    unsigned int high = num_batches;

    while (low < high)
    {
        unsigned int mid = low + (high - low) / 2;
        if (batches[mid].expiration_time < expiration_time)
        {
            low = mid + 1;
        }
        else
        {
            high = mid;
        }
    }

    return low;
}
// Function to add a batch to an ingredient using binary insertion
void add_batch(Ingredient *ingredient, unsigned int quantity, unsigned int expiration_time)
{
    // Find the insertion index using binary search
    unsigned int insert_index = find_insert_index_binary(ingredient->batches, ingredient->num_batches, expiration_time);

    // Resize the batches array if needed
    if (ingredient->num_batches >= ingredient->max_batches)
    {
        ingredient->max_batches++;
        ingredient->batches = realloc(ingredient->batches, ingredient->max_batches * sizeof(Batch));
    }

    // Shift batches to make space for the new batch
    memmove(&ingredient->batches[insert_index + 1], &ingredient->batches[insert_index], (ingredient->num_batches - insert_index) * sizeof(Batch));

    // Add the new batch
    ingredient->batches[insert_index].quantity = quantity;
    ingredient->batches[insert_index].expiration_time = expiration_time;
    ingredient->num_batches++;
}

// Function to add an ingredient with a batch
void add_ingredient(char *name, unsigned int quantity, unsigned int expiration_time)
{
    int index = find_ingredient_index(name);
    if (index != -1)
    {
        // If the ingredient exists, update the total quantity and add the new batch
        ingredients_total[index].total_quantity += quantity;
        add_batch(&ingredients_total[index], quantity, expiration_time);
    }
    else
    {
        // If the ingredient does not exist, resize the total ingredients array if needed
        if (num_ingredients_total >= max_ingredients_total)
        {
            max_ingredients_total++;
            ingredients_total = realloc(ingredients_total, max_ingredients_total * sizeof(Ingredient));
        }
        // Add the new ingredient and the first batch
        strcpy(ingredients_total[num_ingredients_total].name, name);
        ingredients_total[num_ingredients_total].total_quantity = quantity;
        ingredients_total[num_ingredients_total].batches = malloc(sizeof(Batch));
        ingredients_total[num_ingredients_total].num_batches = 0;
        ingredients_total[num_ingredients_total].max_batches = 1; // TODO: check
        add_batch(&ingredients_total[num_ingredients_total], quantity, expiration_time);
        hash_table_insert(ingredient_table, name, num_ingredients_total);
        num_ingredients_total++;
    }
}

// Function to add an ingredient to a recipe (without batches)
void add_ingredient_to_recipe(Ingredient *recipe_ingredient, char *name, unsigned int quantity)
{
    strcpy(recipe_ingredient->name, name);
    recipe_ingredient->total_quantity = quantity;
    recipe_ingredient->batches = NULL; // No batches for ingredients in recipes
    recipe_ingredient->num_batches = 0;
    recipe_ingredient->max_batches = 0;
}

// Function to add a new recipe
void aggiungi_ricetta(char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients, unsigned int max_ingredients)
{
    // Check if the recipe already exists
    if (find_recipe_index(name) != -1)
    {
        printf("ignorato\n");
        return;
    }

    // Resize the recipes array if needed
    if (num_recipes >= max_recipes)
    {
        max_recipes++;
        recipes = realloc(recipes, max_recipes * sizeof(Recipe));
    }

    // Initialize the new recipe
    init_recipe(&recipes[num_recipes], name, num_ingredients);
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        add_ingredient(ingredients[i], quantities[i], time); // Add ingredient to the global list
        add_ingredient_to_recipe(&recipes[num_recipes].ingredients[i], ingredients[i], quantities[i]);
    }

    // Insert the recipe into the hash table
    hash_table_insert(recipe_table, name, num_recipes);
    num_recipes++;
    printf("aggiunta\n");
}

// Function to manage the "aggiungi_ricetta" command
void manage_aggiungi_ricetta(char *line)
{
    // Extract the recipe name from the input line
    char name[WORD_SIZE];
    char *token = strtok(line + 2, " "); // Skip the command "aggiungi_ricetta"
    token = strtok(NULL, " ");           // Extract the recipe name
    strcpy(name, token);

    // Extract the maximum number of ingredients from the input line
    token = strtok(NULL, " ");
    unsigned int max_ingredients = atoi(token);

    // Dynamically allocate the ingredients array
    char **ingredients = malloc(max_ingredients * sizeof(char *));

    // Dynamically allocate the quantities array
    unsigned int *quantities = malloc(max_ingredients * sizeof(unsigned int));

    // Count the number of ingredients
    unsigned int num_ingredients = 0;
    token = strtok(NULL, " ");
    while (token != NULL && num_ingredients < max_ingredients)
    {
        ingredients[num_ingredients] = strdup(token);
        token = strtok(NULL, " ");
        quantities[num_ingredients] = atoi(token);
        token = strtok(NULL, " ");
        num_ingredients++;
    }

    // Call the aggiungi_ricetta function
    aggiungi_ricetta(name, ingredients, quantities, num_ingredients, max_ingredients);

    // Free the memory allocated for the ingredients
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        free(ingredients[i]);
    }
    free(ingredients);
    free(quantities);
}

int main()
{
    ingredient_table = create_table();
    recipe_table = create_table();

    unsigned int current_character;
    // Read the two numbers from the first line
    scanf("%u %u", &courier_frequency, &courier_capacity);
    // Read characters until EOF
    while ((current_character = getchar_unlocked()) != EOF)
    {
        // Check if the current character is a digit
        if (isdigit(current_character))
            continue; // TODO: Verify, probably to delete

        unsigned int max_size = INITIAL_SIZE;
        // Memory allocation for the current line
        char *line = malloc(INITIAL_SIZE * sizeof(char));

        unsigned int current_index = 0;
        // Read characters until end of line or EOF
        while (current_character != '\n' && current_character != EOF && current_character != '\0')
        {
            // Check if we need to expand the array size
            if (current_index >= max_size)
            {
                max_size += WORD_SIZE;
                char *temp = realloc(line, max_size * sizeof(char));
                line = temp;
            }
            line[current_index++] = current_character;
            current_character = getchar_unlocked();
        }

        line[current_index] = '\0'; // Terminate the string with '\0'

        // Process the command based on the third character of the line
        if (current_index >= 3)
            switch (line[2])
            {
            case 'g': // Command to add a recipe
                manage_aggiungi_ricetta(line);
                break;
            case 'm': // Command to remove a recipe (to be implemented)
                // rimuovi_ricetta
                break;
            case 'f': // Command for restocking (to be implemented)
                // rifornimento
                break;
            case 'd': // Command for an order (to be implemented)
                // ordine
                break;
            default:
                break;
            }

        // Free the memory allocated for the current line
        free(line);
    }

    free_table(ingredient_table);
    free_table(recipe_table);

    return 0;
}