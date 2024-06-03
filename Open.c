#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define WORD_SIZE 256    // Size of a single word
#define INITIAL_SIZE 768 // Initial size of the line buffer (WORD_SIZE * 3)

// Global variables
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
    char **ingredient_names;
    unsigned int **ingredient_quantities;
    unsigned int *needed_quantities;
    unsigned int num_ingredients;
} Recipe;

Recipe *recipes = NULL; // Dynamic array of recipes

// O(log n) time, O(1) space
unsigned int find_insert_index_binary(Batch *batches, unsigned int num_batches, unsigned int expiration_time)
{
    unsigned int low = 0;
    unsigned int high = num_batches;

    while (low < high)
    {
        unsigned int mid = low + (high - low) / 2;
        if (batches[mid].expiration_time < expiration_time)
            low = mid + 1;
        else
            high = mid;
    }

    return low;
}

// O(n) time, O(1) space
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

// O(n) time, O(1) space
void add_ingredient(char *name, unsigned int quantity, int expiration_time)
{
    int index = -1;
    for (unsigned int i = 0; i < num_ingredients_total; i++)
    {
        if (strcmp(ingredients_total[i].name, name) == 0)
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        // If the ingredient exists, update the total quantity and add the new batch if expiration_time is specified
        ingredients_total[index].total_quantity += quantity;
        if (expiration_time != -1)
        {
            add_batch(&ingredients_total[index], quantity, expiration_time);
        }
    }
    else
    {
        // If the ingredient does not exist, resize the total ingredients array if needed
        if (num_ingredients_total >= max_ingredients_total)
        {
            max_ingredients_total++;
            ingredients_total = realloc(ingredients_total, max_ingredients_total * sizeof(Ingredient));
        }
        // Add the new ingredient and the first batch if expiration_time is specified
        strcpy(ingredients_total[num_ingredients_total].name, name);
        ingredients_total[num_ingredients_total].total_quantity = quantity;
        ingredients_total[num_ingredients_total].batches = malloc(sizeof(Batch));
        ingredients_total[num_ingredients_total].num_batches = 0;
        ingredients_total[num_ingredients_total].max_batches = 1;
        if (expiration_time != -1)
        {
            add_batch(&ingredients_total[num_ingredients_total], quantity, expiration_time);
        }
        num_ingredients_total++;
    }
}

// O(1) time, O(1) space
void init_recipe(Recipe *recipe, char *name, unsigned int num_ingredients)
{
    strcpy(recipe->name, name);
    recipe->ingredient_names = malloc(num_ingredients * sizeof(char *));
    recipe->ingredient_quantities = malloc(num_ingredients * sizeof(unsigned int *));
    recipe->needed_quantities = malloc(num_ingredients * sizeof(unsigned int));
    recipe->num_ingredients = num_ingredients;
}

// O(n) time, O(1) space
void add_ingredient_to_recipe(Recipe *recipe, unsigned int ingredient_index, char *name, unsigned int *quantity, unsigned int needed_quantity)
{
    recipe->ingredient_names[ingredient_index] = name;
    recipe->ingredient_quantities[ingredient_index] = quantity;
    recipe->needed_quantities[ingredient_index] = needed_quantity;
}

// O(n) time, O(n) space (where n is the number of ingredients in the recipe)
void aggiungi_ricetta(char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{
    // Check if the recipe already exists
    for (unsigned int i = 0; i < num_recipes; i++)
    {
        if (strcmp(recipes[i].name, name) == 0)
        {
            printf("ignorato\n");
            return;
        }
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
        // Add ingredient to the global list without specifying expiration time
        add_ingredient(ingredients[i], 0, -1);
        
        // Find the ingredient in the global list
        unsigned int j;
        for (j = 0; j < num_ingredients_total; j++)
        {
            if (strcmp(ingredients_total[j].name, ingredients[i]) == 0)
            {
                break;
            }
        }
        
        add_ingredient_to_recipe(&recipes[num_recipes], i, ingredients_total[j].name, &ingredients_total[j].total_quantity, quantities[i]);
    }

    num_recipes++;
    printf("aggiunta\n");
}

// O(n) time, O(n) space (where n is the length of the input line)
void manage_aggiungi_ricetta(char *line)
{
    // Skip the command "aggiungi_ricetta"
    char *token = strtok(line + strlen("aggiungi_ricetta") + 1, " ");
    // Extract the recipe name
    char *name = token;

    // Initialize variables to store ingredient information
    char **ingredients = NULL;
    unsigned int *quantities = NULL;
    unsigned int max_ingredients = 0;
    unsigned int num_ingredients = 0;

    // Parse the rest of the string to extract ingredient names and quantities
    while ((token = strtok(NULL, " ")) != NULL)
    {
        // Check if we have enough space in the arrays, if not, resize them
        if (num_ingredients >= max_ingredients)
        {
            max_ingredients += 1;
            ingredients = realloc(ingredients, max_ingredients * sizeof(char *));
            quantities = realloc(quantities, max_ingredients * sizeof(unsigned int));
        }

        // Extract ingredient name and quantity
        char *ingredient_name = token;
        token = strtok(NULL, " ");
        unsigned int quantity = atoi(token);

        // Add the ingredient to the arrays
        ingredients[num_ingredients] = ingredient_name;
        quantities[num_ingredients] = quantity;
        num_ingredients++;
    }

    // Call the aggiungi_ricetta function
    printf("Chiamata aggiungi_ricetta con i seguenti parametri:\n");
    printf("Nome ricetta: %s\n", name);
    printf("Ingredienti:\n");
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        printf("Ingrediente %u: %s, Quantità: %u\n", i + 1, ingredients[i], quantities[i]);
    }
    aggiungi_ricetta(name, ingredients, quantities, num_ingredients);

    // Free the memory allocated for the ingredients
    free(ingredients);
    free(quantities);
}




// O(n) time, O(1) space
void print_ingredient_table()
{
    printf("Ingredient Table:\n");
    printf("Numero ingredienti: %u\n", num_ingredients_total);
    for (unsigned int i = 0; i < num_ingredients_total; i++)
    {
        printf("Ingredient: %s, Total Quantity: %u", ingredients_total[i].name, ingredients_total[i].total_quantity);
        if (ingredients_total[i].num_batches > 0)
        {
            printf(", Batches:\n");
            for (unsigned int j = 0; j < ingredients_total[i].num_batches; j++)
            {
                printf("  Quantity: %u, Expiration Time: %u\n", ingredients_total[i].batches[j].quantity, ingredients_total[i].batches[j].expiration_time);
            }
        }
        else
        {
            printf(", No batches\n");
        }
    }
}

// O(n) time, O(1) space
void print_recipe_table()
{
    printf("Recipe Table:\n");
    for (unsigned int i = 0; i < num_recipes; i++)
    {
        printf("Recipe: %s, Ingredients:\n", recipes[i].name);
        for (unsigned int j = 0; j < recipes[i].num_ingredients; j++)
        {
            printf("  Ingredient: %s, Quantity: %u\n", recipes[i].ingredient_names[j], recipes[i].needed_quantities[j]);
        }
    }
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
        if (isdigit(current_character)) // TODO: check, maybe delete useless condition
            continue;

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
        if (current_index >= 3) // TODO: check, maybe delete useless condition
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

    // Print the contents of the ingredient table
    add_ingredient("tuorlo", 100, 150);
    print_ingredient_table();

    // Print the contents of the recipe table
    print_recipe_table();

    // Free the dynamic memory allocations
    for (unsigned int i = 0; i < num_ingredients_total; i++)
        free(ingredients_total[i].batches);
    free(ingredients_total);
    for (unsigned int i = 0; i < num_recipes; i++)
    {
        free(recipes[i].ingredient_names);
        free(recipes[i].ingredient_quantities);
        free(recipes[i].needed_quantities);
    }
    free(recipes);

    return 0;
}
