#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define WORD_SIZE 256    // Size of a single word
#define INITIAL_SIZE 768 // Initial size of the line buffer (WORD_SIZE * 3)

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

// Function to find the index of a recipe by name
unsigned int find_recipe_index(char *name)
{
    for (unsigned int i = 0; i < num_recipes; i++)
        if (strcmp(recipes[i].name, name) == 0)
            return i;
    return -1;
}

// Function to find the index of an ingredient by name
unsigned int find_ingredient_index(char *name)
{
    for (unsigned int i = 0; i < num_ingredients_total; i++)
        if (strcmp(ingredients_total[i].name, name) == 0)
            return i;
    return -1;
}

// Function to initialize a new recipe
void init_recipe(Recipe *recipe, char *name, unsigned int num_ingredients)
{
    strcpy(recipe->name, name);
    recipe->ingredients = malloc(num_ingredients * sizeof(Ingredient));
    recipe->num_ingredients = num_ingredients;
}

// Function to compare batches by expiration time for sorting
int compare_batches(const void *a, const void *b)
{
    Batch *batchA = (Batch *)a;
    Batch *batchB = (Batch *)b;
    return (batchA->expiration_time - batchB->expiration_time);
}

// Function to add a batch to an ingredient
void add_batch(Ingredient *ingredient, unsigned int quantity, unsigned int expiration_time)
{
    // Resize the batches array if needed
    if (ingredient->num_batches >= ingredient->max_batches)
    {
        ingredient->max_batches++;
        ingredient->batches = realloc(ingredient->batches, ingredient->max_batches * sizeof(Batch));
    }
    // Add the new batch
    Batch new_batch = {quantity, expiration_time};
    ingredient->batches[ingredient->num_batches++] = new_batch;
    // Sort batches by expiration time using QuickSort
    qsort(ingredient->batches, ingredient->num_batches, sizeof(Batch), compare_batches);
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
        ingredients_total[num_ingredients_total].max_batches = 1;
        add_batch(&ingredients_total[num_ingredients_total], quantity, expiration_time);
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
        add_ingredient_to_recipe(&recipes[num_recipes].ingredients[i], ingredients[i], quantities[i]);

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

    return 0;
}