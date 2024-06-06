#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define WORD_SIZE 256          // Size of a single word
#define INITIAL_SIZE 768       // Initial size of the line buffer (WORD_SIZE * 3)
unsigned int time_elapsed = 0; // Global variable
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
Ingredient *ingredients_total = NULL; // Binary tree of all ingredients
// Structure to represent a recipe
typedef struct Recipe
{
    char name[WORD_SIZE];
    Ingredient **ingredient_pointers;
    unsigned int *needed_quantities;
    unsigned int weight;
    unsigned int num_ingredients;
    struct Recipe *left;
    struct Recipe *right;
} Recipe;
Recipe *recipes = NULL; // Binary tree of all recipes
// Structure to represent a node in the recipe queue
typedef struct RecipeNode
{
    Recipe *recipe;
    unsigned int quantity;
    unsigned int arrival_time;
    struct RecipeNode *next;
} RecipeNode;
// Structure to represent the recipe queue
typedef struct RecipeQueue
{
    RecipeNode *front;
    RecipeNode *rear;
} RecipeQueue;
RecipeQueue order_queue = {NULL, NULL};           // Queue for managing orders
RecipeQueue completed_order_queue = {NULL, NULL}; // Queue for managing completed orders
// Time complexity: O(log n), space complexity: O(1)
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
// Time complexity: O(1), space complexity: O(1)
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
// Time complexity: O(log n), space complexity: O(1)
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
// Time complexity: O(1), space complexity: O(n)
Recipe *create_recipe(char *name, unsigned int num_ingredients)
{
    Recipe *new_recipe = malloc(sizeof(Recipe));
    strcpy(new_recipe->name, name);
    new_recipe->ingredient_pointers = malloc(num_ingredients * sizeof(Ingredient *));
    new_recipe->needed_quantities = malloc(num_ingredients * sizeof(unsigned int));
    new_recipe->num_ingredients = num_ingredients;
    new_recipe->weight = 0;
    new_recipe->left = NULL;
    new_recipe->right = NULL;
    return new_recipe;
}
// Time complexity: O(log n), space complexity: O(n)
Recipe *insert_recipe(Recipe *root, char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{
    if (root == NULL)
        return create_recipe(name, num_ingredients);
    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        root->left = insert_recipe(root->left, name, ingredients, quantities, num_ingredients);
    else if (cmp > 0)
        root->right = insert_recipe(root->right, name, ingredients, quantities, num_ingredients);
    else
    {
        printf("ignorato\n");
        return root;
    }
    return root;
}
// Time complexity: Depends on the number of ingredients in the recipe, space complexity: O(n)
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
// Time complexity: O(log n), space complexity: O(1)
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
// Time complexity: O(n log n), space complexity: O(n)
void aggiungi_ricetta(char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{
    recipes = insert_recipe(recipes, name, ingredients, quantities, num_ingredients);
    Recipe *new_recipe = find_recipe(recipes, name);
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        ingredients_total = rifornimento(ingredients_total, ingredients[i], 0, -1);
        Ingredient *current = find_ingredient(ingredients_total, ingredients[i]);
        new_recipe->ingredient_pointers[i] = current;
        new_recipe->needed_quantities[i] = quantities[i];
        new_recipe->weight += quantities[i];
    }
    printf("aggiunta\n");
}
// Time complexity: Depends on the number of ingredients in the recipe, space complexity: O(n)
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
// Time complexity: O(n), space complexity: O(1)
int can_fulfill_order(Recipe *recipe, unsigned int order_quantity)
{
    for (unsigned int i = 0; i < recipe->num_ingredients; i++)
        if (recipe->ingredient_pointers[i]->total_quantity < recipe->needed_quantities[i] * order_quantity)
            return 0;
    return 1;
}
// Time complexity: O(n + m), space complexity: O(1)
void fulfill_order(Recipe *recipe, unsigned int order_quantity)
{
    for (unsigned int i = 0; i < recipe->num_ingredients; i++)
    {
        unsigned int required_quantity = recipe->needed_quantities[i] * order_quantity;
        Ingredient *ingredient = recipe->ingredient_pointers[i];
        ingredient->total_quantity -= required_quantity;
        for (unsigned int j = 0; j < ingredient->num_batches && required_quantity > 0; j++)
            if (ingredient->batches[j].quantity > required_quantity)
            {
                ingredient->batches[j].quantity -= required_quantity;
                required_quantity = 0;
            }
            else
            {
                required_quantity -= ingredient->batches[j].quantity;
                ingredient->batches[j].quantity = 0;
            }
        unsigned int new_num_batches = 0;
        for (unsigned int j = 0; j < ingredient->num_batches; j++)
            if (ingredient->batches[j].quantity > 0)
                ingredient->batches[new_num_batches++] = ingredient->batches[j];
        ingredient->num_batches = new_num_batches;
        if (ingredient->num_batches < ingredient->max_batches)
        {
            ingredient->max_batches = ingredient->num_batches;
            ingredient->batches = realloc(ingredient->batches, ingredient->max_batches * sizeof(Batch));
        }
    }
}
// Time complexity: O(1), space complexity: O(1)
void add_order_to_queue(Recipe *recipe, unsigned int quantity)
{
    printf("accettato\n");
    if (can_fulfill_order(recipe, quantity))
    {
        fulfill_order(recipe, quantity);
        printf("ordine completato: %s x%u\n", recipe->name, quantity);
        RecipeNode *new_node = malloc(sizeof(RecipeNode));
        new_node->recipe = recipe;
        new_node->quantity = quantity;
        new_node->next = NULL;
        if (completed_order_queue.rear == NULL)
            completed_order_queue.front = new_node;
        else
            completed_order_queue.rear->next = new_node;
        completed_order_queue.rear = new_node;
    }
    else
    {
        RecipeNode *new_node = malloc(sizeof(RecipeNode));
        new_node->recipe = recipe;
        new_node->quantity = quantity;
        new_node->arrival_time = time_elapsed;
        new_node->next = NULL;
        if (order_queue.rear == NULL)
            order_queue.front = new_node;
        else
            order_queue.rear->next = new_node;
        order_queue.rear = new_node;
    }
}

// Time complexity: O(n), space complexity: O(n). Function to process the queue and fulfill orders
void ordine()
{
    RecipeNode *prev = NULL;
    RecipeNode *current = order_queue.front;
    while (current != NULL)
    {
        if (can_fulfill_order(current->recipe, current->quantity))
        {
            fulfill_order(current->recipe, current->quantity);
            printf("ordine completato: %s x%u\n", current->recipe->name, current->quantity); // TODO: delete after
            // Add the completed order to the completed order queue
            RecipeNode *new_node = malloc(sizeof(RecipeNode));
            new_node->recipe = current->recipe;
            new_node->quantity = current->quantity;
            new_node->next = NULL;
            if (completed_order_queue.rear == NULL)
                completed_order_queue.front = new_node;
            else
                completed_order_queue.rear->next = new_node;
            completed_order_queue.rear = new_node;
            RecipeNode *temp = current;
            current = current->next;
            if (prev == NULL)
                order_queue.front = current;
            else
                prev->next = current;
            free(temp);
            if (order_queue.front == NULL)
                order_queue.rear = NULL;
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }
}

// Time complexity: Depends on the length of the input line, space complexity: O(1). Function to manage orders
void manage_ordine(char *line)
{
    char *recipe_name = strtok(line + strlen("ordine") + 1, " ");
    unsigned int quantity = atoi(strtok(NULL, " "));
    Recipe *recipe = find_recipe(recipes, recipe_name);
    if (recipe == NULL)
    {
        printf("rifiutato\n");
        return;
    }

    add_order_to_queue(recipe, quantity);
}
// Time complexity: O(n), space complexity: O(1). Function to check if a recipe is present in a queue
int recipe_in_queue(RecipeQueue queue, char *name)
{
    RecipeNode *current = queue.front;
    while (current != NULL)
    {
        if (strcmp(current->recipe->name, name) == 0)
        {
            printf("ordini in sospeso\n");
            return 1;
        }
        current = current->next;
    }
    return 0;
}

// Time complexity: O(n), space complexity: O(n). Function to remove a recipe from the binary tree
Recipe *rimuovi_ricetta(Recipe *root, char *name)
{
    if (root == NULL)
        return NULL;
    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        root->left = rimuovi_ricetta(root->left, name);
    else if (cmp > 0)
        root->right = rimuovi_ricetta(root->right, name);
    else
    {
        if ((order_queue.front != NULL && recipe_in_queue(order_queue, name)) || (completed_order_queue.front != NULL && recipe_in_queue(completed_order_queue, name))) // Check if the recipe is present in any pending orders or completed orders
            return root;
        printf("rimossa\n");
        free(root->ingredient_pointers);
        free(root->needed_quantities);
        free(root);
        return NULL;
    }
    return root;
}

// Time complexity: O(n), space complexity: O(1). Function to manage removing a recipe
void manage_rimuovi_ricetta(char *line)
{
    char *token = strtok(line + strlen("rimuovi_ricetta") + 1, " ");
    char *recipe_name = token;
    Recipe *found_recipe = find_recipe(recipes, recipe_name);
    if (found_recipe == NULL)
    {
        printf("non presente\n");
        return;
    }
    // Remove the recipe
    recipes = rimuovi_ricetta(recipes, recipe_name);
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
    free(root->ingredient_pointers);
    free(root->needed_quantities);
    free(root);
}

void print_ingredients(Ingredient *root) // TODO: delete after ending
{
    if (root == NULL)
        return;

    print_ingredients(root->left);

    printf("Ingredient: %s, Total Quantity: %u", root->name, root->total_quantity);
    if (root->num_batches > 0)
    {
        printf(", Batches:\n");
        for (unsigned int j = 0; j < root->num_batches; j++)
            printf("  Quantity: %u, Expiration Time: %u\n", root->batches[j].quantity, root->batches[j].expiration_time);
    }
    else
        printf(", No batches\n");

    print_ingredients(root->right);
}
void print_recipes(Recipe *root) // TODO: delete after ending
{
    if (root == NULL)
        return;

    print_recipes(root->left);

    printf("Ricetta: %s\n", root->name);
    printf("  Ingredienti:\n");
    for (unsigned int j = 0; j < root->num_ingredients; j++)
        printf("    %s: %u\n", root->ingredient_pointers[j]->name, root->needed_quantities[j]);

    print_recipes(root->right);
}
void print_order_queue(RecipeQueue queue) // TODO: delete after ending
{
    RecipeNode *current = queue.front;

    if (current == NULL)
    {
        printf("Order queue is empty\n");
        return;
    }

    printf("Order queue:\n");
    while (current != NULL)
    {
        printf("Recipe: %s, Quantity: %u\n", current->recipe->name, current->quantity);
        current = current->next;
    }
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
    printf("rifornito\n");

    // Check completed orders
    ordine();
}
// Time complexity: O(n), space complexity: O(1). Function to remove expired batches from an ingredient
void remove_spoiled_batches(Ingredient *ingredient)
{
    unsigned int write_index = 0;
    for (unsigned int i = 0; i < ingredient->num_batches; i++)
    {
        if (time_elapsed < ingredient->batches[i].expiration_time)
        {
            ingredient->batches[write_index] = ingredient->batches[i];
            write_index++;
        }
        else
        {
            ingredient->total_quantity -= ingredient->batches[i].quantity;
        }
    }
    ingredient->num_batches = write_index;
}

// Time complexity: O(n), space complexity: O(1). Function to traverse the binary tree of ingredients and remove expired batches
void remove_spoiled_batches_from_tree(Ingredient *root)
{
    if (root == NULL)
        return;
    remove_spoiled_batches_from_tree(root->left);
    remove_spoiled_batches(root);
    remove_spoiled_batches_from_tree(root->right);
}

int main()
{
    unsigned int current_character, courier_frequency, courier_capacity;
    // Read the two numbers from the first line
    scanf("%u %u", &courier_frequency, &courier_capacity);
    // Read characters until EOF
    while ((current_character = getchar_unlocked()) != EOF)
    {
        if (isdigit(current_character)) // TODO: delete if useless
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

        if (current_index >= 3) // TODO: delete if useless
        {
            switch (line[2])
            {
            case 'g':
                manage_aggiungi_ricetta(line);
                break;
            case 'm':
                manage_rimuovi_ricetta(line);
                break;
            case 'f':
                manage_rifornimento(line);
                break;
            case 'd':
                manage_ordine(line);
                break;
            default:
                break;
            }
        }
        time_elapsed++;
        remove_spoiled_batches_from_tree(ingredients_total); // TODO: check if this slows too much the code
        // TODO: manage courier
        free(line);
    }

    print_ingredients(ingredients_total);     // TODO: delete after ending
    print_recipes(recipes);                   // TODO: delete after ending
    print_order_queue(order_queue);           // TODO: delete after ending
    print_order_queue(completed_order_queue); // TODO: delete after ending

    free_ingredients(ingredients_total);
    free_recipes(recipes);

    return 0;
}
