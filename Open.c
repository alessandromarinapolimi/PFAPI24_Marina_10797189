#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define WORD_SIZE 256    // Size of a single word
#define INITIAL_SIZE 768 // Initial size of the line buffer (WORD_SIZE * 3)

// Global variables
unsigned int courier_frequency, courier_capacity, num_recipes = 0, max_recipes = 0, time_elapsed = 0;

// Structure to represent a batch of an ingredient
typedef struct
{
    unsigned int quantity;
    unsigned int expiration_time;
} Batch;

// Structure to represent an ingredient
typedef struct Ingredient
{
    char name[WORD_SIZE];
    unsigned int total_quantity;
    Batch *batches;
    unsigned int num_batches;
    unsigned int max_batches;
    struct Ingredient *left;
    struct Ingredient *right;
} Ingredient;

Ingredient *ingredients_total = NULL; // Binary tree of total ingredients

// Structure to represent a recipe
typedef struct Recipe
{
    char name[WORD_SIZE];
    unsigned int **ingredient_quantities;
    unsigned int *needed_quantities;
    unsigned int num_ingredients;
    struct Recipe *left;
    struct Recipe *right;
} Recipe;

Recipe *recipes = NULL; // Binary tree of recipes

// Time complexity: O(log n), space complexity: O(1).  Function to compare batches by expiration time using binary insertion
unsigned int find_insert_index_binary_batch(Batch *batches, unsigned int num_batches, unsigned int expiration_time)
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

// Time complexity: O(1), space complexity: O(1).  Function to create a new ingredient node
Ingredient *create_ingredient(char *name, unsigned int quantity, int expiration_time)
{
    Ingredient *new_ingredient = malloc(sizeof(Ingredient));
    strcpy(new_ingredient->name, name);
    new_ingredient->total_quantity = quantity;
    new_ingredient->batches = malloc(sizeof(Batch));
    new_ingredient->num_batches = 0;
    new_ingredient->max_batches = 1;
    new_ingredient->left = NULL;
    new_ingredient->right = NULL;
    if (expiration_time != -1)
    {
        new_ingredient->batches[0].quantity = quantity;
        new_ingredient->batches[0].expiration_time = expiration_time + time_elapsed;
        new_ingredient->num_batches = 1;
    }
    return new_ingredient;
}

// Time complexity: O(log n) in average case, O(n) in worst case, space complexity: O(1). Function to insert an ingredient into the binary tree
Ingredient *rifornimento(Ingredient *root, char *name, unsigned int quantity, int expiration_time)
{
    if (root == NULL)
        return create_ingredient(name, quantity, expiration_time);

    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        root->left = rifornimento(root->left, name, quantity, expiration_time);
    else if (cmp > 0)
        root->right = rifornimento(root->right, name, quantity, expiration_time);
    else
    {
        // If the ingredient already exists, update its quantity and add a batch if expiration_time is specified
        root->total_quantity += quantity;
        if (expiration_time != -1)
        {
            unsigned int insert_index = find_insert_index_binary_batch(root->batches, root->num_batches, expiration_time + time_elapsed);
            if (root->num_batches >= root->max_batches)
            {
                root->max_batches++;
                root->batches = realloc(root->batches, root->max_batches * sizeof(Batch));
            }
            memmove(&root->batches[insert_index + 1], &root->batches[insert_index], (root->num_batches - insert_index) * sizeof(Batch));
            root->batches[insert_index].quantity = quantity;
            root->batches[insert_index].expiration_time = expiration_time + time_elapsed;
            root->num_batches++;
        }
    }

    return root;
}

// Time complexity: O(1), space complexity: O(1). Function to create a new recipe node
Recipe *create_recipe(char *name, unsigned int num_ingredients)
{
    Recipe *new_recipe = malloc(sizeof(Recipe));
    strcpy(new_recipe->name, name);
    new_recipe->ingredient_quantities = malloc(num_ingredients * sizeof(unsigned int *));
    new_recipe->needed_quantities = malloc(num_ingredients * sizeof(unsigned int));
    new_recipe->num_ingredients = num_ingredients;
    new_recipe->left = NULL;
    new_recipe->right = NULL;
    return new_recipe;
}

// Time complexity: O(log n) in average case, O(n) in worst case, space complexity: O(1). Function to insert a recipe into the binary tree
Recipe *insert_recipe(Recipe *root, char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{
    if (root == NULL)
        return create_recipe(name, num_ingredients);
    // TODO: check
    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        root->left = insert_recipe(root->left, name, ingredients, quantities, num_ingredients);
    else if (cmp > 0)
        root->right = insert_recipe(root->right, name, ingredients, quantities, num_ingredients);
    else
    {
        // TODO: check, maybe delete useless condition because its repetitive
        printf("ignorato\n");
        return root;
    }

    return root;
}

// Time complexity: O(log n) in average case, O(n) in worst case, space complexity: O(1). Function to find an ingredient in the binary tree
Ingredient *find_ingredient(Ingredient *root, char *name)
{
    if (root == NULL)
        return NULL;

    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        return find_ingredient(root->left, name);
    else if (cmp > 0)
        return find_ingredient(root->right, name);
    else
        return root;
}

// Time complexity: O(log n) in average case, O(n) in worst case, space complexity: O(1). Function to find a recipe in the binary tree
Recipe *find_recipe(Recipe *root, char *name)
{
    if (root == NULL)
        return NULL;

    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        return find_recipe(root->left, name);
    else if (cmp > 0)
        return find_recipe(root->right, name);
    else
        return root;
}

// Time complexity: O(log n) in average case, O(n) in worst case, space complexity: O(1). Function to add a new recipe
void aggiungi_ricetta(char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{
    if (find_recipe(recipes, name) != NULL)
    {
        printf("ignorato\n");
        return;
    }

    recipes = insert_recipe(recipes, name, ingredients, quantities, num_ingredients);

    Recipe *new_recipe = find_recipe(recipes, name);
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        ingredients_total = rifornimento(ingredients_total, ingredients[i], 0, -1);

        Ingredient *current = find_ingredient(ingredients_total, ingredients[i]);
        new_recipe->ingredient_quantities[i] = &current->total_quantity;
        new_recipe->needed_quantities[i] = quantities[i];
    }

    num_recipes++;
    printf("aggiunta\n");
}

// Time complexity: Depends on the length of the input line, space complexity: O(1). Function to manage adding a recipe
void manage_aggiungi_ricetta(char *line)
{
    char *token = strtok(line + strlen("aggiungi_ricetta") + 1, " ");
    char *name = token;

    char **ingredients = NULL;
    unsigned int *quantities = NULL;
    unsigned int max_ingredients = 0;
    unsigned int num_ingredients = 0;

    while ((token = strtok(NULL, " ")) != NULL)
    {
        if (num_ingredients >= max_ingredients)
        {
            max_ingredients += 1;
            ingredients = realloc(ingredients, max_ingredients * sizeof(char *));
            quantities = realloc(quantities, max_ingredients * sizeof(unsigned int));
        }

        char *ingredient_name = token;
        token = strtok(NULL, " ");
        unsigned int quantity = atoi(token);

        ingredients[num_ingredients] = ingredient_name;
        quantities[num_ingredients] = quantity;
        num_ingredients++;
    }

    aggiungi_ricetta(name, ingredients, quantities, num_ingredients);

    free(ingredients);
    free(quantities);
}

// Time complexity: Depends on the length of the input line, space complexity: O(1). Function to manage restocking
void manage_rifornimento(char *line)
{
    char *token = strtok(line + strlen("rifornimento") + 1, " ");

    while (token != NULL)
    {
        char *ingredient_name = token;
        token = strtok(NULL, " ");
        if (token == NULL)
            break;
        unsigned int quantity = atoi(token);
        token = strtok(NULL, " ");
        if (token == NULL)
            break;
        unsigned int expiration_time = atoi(token);

        ingredients_total = rifornimento(ingredients_total, ingredient_name, quantity, expiration_time);

        token = strtok(NULL, " ");
    }
}

// Time complexity: O(n), space complexity: O(1). Function to print the ingredient table
void print_ingredient_table(Ingredient *root) // TODO: delete after ending
{
    if (root == NULL)
        return;

    print_ingredient_table(root->left);

    printf("Ingredient: %s, Total Quantity: %u", root->name, root->total_quantity);
    if (root->num_batches > 0)
    {
        printf(", Batches:\n");
        for (unsigned int j = 0; j < root->num_batches; j++)
            printf("  Quantity: %u, Expiration Time: %u\n", root->batches[j].quantity, root->batches[j].expiration_time);
    }
    else
        printf(", No batches\n");

    print_ingredient_table(root->right);
}

// Time complexity: O(n), space complexity: O(1). Function to print the recipe table
void print_recipe_table(Recipe *root) // TODO: delete after ending
{
    if (root == NULL)
        return;

    print_recipe_table(root->left);

    printf("Ricetta: %s\n", root->name);
    printf("  Ingredienti:\n");
    for (unsigned int j = 0; j < root->num_ingredients; j++)
        printf("    %p: %u\n", (void *)root->ingredient_quantities[j], root->needed_quantities[j]);

    print_recipe_table(root->right);
}

// Time complexity: O(n), space complexity: O(n). Function to free the memory allocated for the ingredients
void free_ingredients(Ingredient *root)
{
    if (root == NULL)
        return;

    free_ingredients(root->left);
    free_ingredients(root->right);

    free(root->batches);
    free(root);
}

// Time complexity: O(n), space complexity: O(n). Function to free the memory allocated for the recipes
void free_recipes(Recipe *root)
{
    if (root == NULL)
        return;

    free_recipes(root->left);
    free_recipes(root->right);

    free(root->ingredient_quantities);
    free(root->needed_quantities);
    free(root);
}

int main()
{
    unsigned int current_character;

    // Read the two numbers from the first line
    scanf("%u %u", &courier_frequency, &courier_capacity);
    // Read characters until EOF
    while ((current_character = getchar_unlocked()) != EOF)
    {
        if (isdigit(current_character)) // TODO: check, maybe delete useless condition
            continue;

        unsigned int max_size = INITIAL_SIZE;
        char *line = malloc(INITIAL_SIZE * sizeof(char));

        unsigned int current_index = 0;
        while (current_character != '\n' && current_character != EOF && current_character != '\0')
        {
            if (current_index >= max_size)
            {
                max_size += WORD_SIZE;
                char *temp = realloc(line, max_size * sizeof(char));
                line = temp;
            }
            line[current_index++] = current_character;
            current_character = getchar_unlocked();
        }

        line[current_index] = '\0';

        if (current_index >= 3)
        {
            switch (line[2])
            {
            case 'g':
                manage_aggiungi_ricetta(line);
                break;
            case 'm':
                // rimuovi_ricetta
                break;
            case 'f':
                manage_rifornimento(line);
                break;
            case 'd':
                // ordine
                break;
            default:
                break;
            }
        }

        free(line);
    }

    print_ingredient_table(ingredients_total); // TODO: delete after ending
    print_recipe_table(recipes);               // TODO: delete after ending

    free_ingredients(ingredients_total);
    free_recipes(recipes);

    return 0;
}
