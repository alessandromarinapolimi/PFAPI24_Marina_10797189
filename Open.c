#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#define WORD_SIZE 21     // Size of a word
#define INITIAL_SIZE 252 // Initial size of the line buffer (WORD_SIZE * 3)
unsigned int time_elapsed = 0;
typedef struct
{
    unsigned int quantity;
    unsigned int expiration_time;
} Batch;
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
Ingredient *create_ingredient(char *name, unsigned int quantity, unsigned int expiration_time)
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
        new_ingredient->batches[0].expiration_time = expiration_time;
        new_ingredient->num_batches = 1;
    }
    return new_ingredient;
}
// Time complexity: O(log n), space complexity: O(1)
Ingredient *rifornimento(Ingredient *root, char *name, unsigned int quantity, unsigned int expiration_time)
{
    if (root == NULL)
        return create_ingredient(name, quantity, expiration_time);
    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        root->left = rifornimento(root->left, name, quantity, expiration_time);
    else if (cmp > 0)
        root->right = rifornimento(root->right, name, quantity, expiration_time);
    else if (quantity > 0 && expiration_time > time_elapsed)
    {
        root->total_quantity += quantity;
        unsigned int insert_index = find_insert_index_binary_batch(root->batches, root->num_batches, expiration_time);
        if (root->num_batches >= root->max_batches)
        {
            root->max_batches++;
            root->batches = realloc(root->batches, root->max_batches * sizeof(Batch));
        }
        memmove(&root->batches[insert_index + 1], &root->batches[insert_index], (root->num_batches - insert_index) * sizeof(Batch));
        root->batches[insert_index].quantity = quantity;
        root->batches[insert_index].expiration_time = expiration_time;
        root->num_batches++;
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
// Time complexity: O(1), space complexity: O(n)
Recipe *create_recipe(char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{

    Recipe *new_recipe = malloc(sizeof(Recipe));
    strcpy(new_recipe->name, name);
    new_recipe->ingredient_pointers = malloc(num_ingredients * sizeof(Ingredient *));
    new_recipe->needed_quantities = malloc(num_ingredients * sizeof(unsigned int));
    new_recipe->num_ingredients = num_ingredients;
    new_recipe->weight = 0;
    new_recipe->left = NULL;
    new_recipe->right = NULL;
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        ingredients_total = rifornimento(ingredients_total, ingredients[i], 0, 0);
        new_recipe->ingredient_pointers[i] = find_ingredient(ingredients_total, ingredients[i]);
        new_recipe->needed_quantities[i] = quantities[i];
        new_recipe->weight += quantities[i];
    }
    return new_recipe;
}
// Time complexity: O(log n), space complexity: O(n)
bool insert_recipe(Recipe **root, char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients)
{
    if (*root == NULL)
    {
        *root = create_recipe(name, ingredients, quantities, num_ingredients);
        return 1;
    }
    int cmp = strcmp(name, (*root)->name);
    if (cmp < 0)
        return insert_recipe(&((*root)->left), name, ingredients, quantities, num_ingredients);
    else if (cmp > 0)
        return insert_recipe(&((*root)->right), name, ingredients, quantities, num_ingredients);
    else
    {
        printf("ignorato\n");
        return 0;
    }
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
    if (insert_recipe(&recipes, name, ingredients, quantities, num_ingredients))
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
bool can_fulfill_order(Recipe *recipe, unsigned int order_quantity)
{
    for (unsigned int i = 0; i < recipe->num_ingredients; i++)
        if (recipe->ingredient_pointers[i]->total_quantity < recipe->needed_quantities[i] * order_quantity)
            return false;
    return true;
}
// Time complexity: O(n + m), space complexity: O(1)
void fulfill_order(Recipe *recipe, unsigned int order_quantity)
{
    for (unsigned int i = 0; i < recipe->num_ingredients; i++)
    {
        unsigned int required_quantity = recipe->needed_quantities[i] * order_quantity;
        Ingredient *ingredient = recipe->ingredient_pointers[i];
        // Decrement the total quantity of the ingredient
        ingredient->total_quantity -= required_quantity;
        // Iterate through batches and reduce the required quantity
        unsigned int new_num_batches = 0;
        for (unsigned int j = 0; j < ingredient->num_batches; j++)
            if (ingredient->batches[j].quantity > required_quantity)
            {
                ingredient->batches[j].quantity -= required_quantity;
                required_quantity = 0;
                ingredient->batches[new_num_batches++] = ingredient->batches[j];
            }
            else
            {
                required_quantity -= ingredient->batches[j].quantity;
                ingredient->batches[j].quantity = 0;
            }
        // Update the number of batches and resize if necessary
        ingredient->num_batches = new_num_batches;
        if (ingredient->num_batches < ingredient->max_batches)
        {
            ingredient->max_batches = ingredient->num_batches;
            ingredient->batches = realloc(ingredient->batches, ingredient->max_batches * sizeof(Batch));
        }
    }
}
// Time complexity: O(n), space complexity: O(1).
RecipeNode *find_insert_queue_position(RecipeNode *head, RecipeNode *new_node)
{
    RecipeNode *low = NULL;
    RecipeNode *high = head;
    while (high != NULL)
    {
        if (high->arrival_time < new_node->arrival_time)
        {
            low = high;
            high = high->next;
        }
        else
            break;
    }
    return low;
}
// Time complexity: O(n), space complexity: O(1). Function to remove expired batches from an ingredient
void remove_spoiled_batches(Ingredient *ingredient)
{
    unsigned int write_index = 0;
    for (unsigned int i = 0; i < ingredient->num_batches; i++)
        if (time_elapsed < ingredient->batches[i].expiration_time)
            ingredient->batches[write_index++] = ingredient->batches[i];
        else
            ingredient->total_quantity -= ingredient->batches[i].quantity;
    ingredient->num_batches = write_index;
    if (ingredient->num_batches < ingredient->max_batches)
    {
        ingredient->max_batches = ingredient->num_batches;
        if (ingredient->num_batches > 0)
            ingredient->batches = realloc(ingredient->batches, ingredient->max_batches * sizeof(Batch));
        else
        {
            free(ingredient->batches);
            ingredient->batches = NULL;
        }
    }
}
// Time complexity: O(n), space complexity: O(1). Function to traverse the binary tree of ingredients and remove expired batches
void remove_spoiled_batches_from_tree(Ingredient *root)
{
    if (root == NULL)
        return;
    remove_spoiled_batches_from_tree(root->left);
    remove_spoiled_batches_from_tree(root->right);
    remove_spoiled_batches(root);
}
// Time complexity: O(n), space complexity: O(1). Function to process the queue and fulfill orders
void ordine(Recipe *recipe, unsigned int quantity)
{
    remove_spoiled_batches_from_tree(ingredients_total);
    RecipeNode *new_node = malloc(sizeof(RecipeNode));
    new_node->recipe = recipe;
    new_node->quantity = quantity;
    new_node->arrival_time = time_elapsed;
    new_node->next = NULL;
    // Check if the recipe can be fulfilled
    if (can_fulfill_order(recipe, quantity))
    {
        // Fulfill the order
        fulfill_order(recipe, quantity);
        // Insert into completed_order_queue in the correct position
        RecipeNode *insert_pos = find_insert_queue_position(completed_order_queue.front, new_node);
        if (insert_pos == NULL)
        {
            new_node->next = completed_order_queue.front;
            completed_order_queue.front = new_node;
            if (completed_order_queue.rear == NULL)
                completed_order_queue.rear = new_node;
        }
        else
        {
            new_node->next = insert_pos->next;
            insert_pos->next = new_node;
            if (new_node->next == NULL)
                completed_order_queue.rear = new_node;
        }
    }
    else
    {
        // Add to order_queue if the recipe cannot be fulfilled
        if (order_queue.front == NULL)
        {
            order_queue.front = new_node;
            order_queue.rear = new_node;
        }
        else
        {
            order_queue.rear->next = new_node;
            order_queue.rear = new_node;
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
    printf("accettato\n");
    ordine(recipe, quantity);
}
// Time complexity: O(n), space complexity: O(1). Function to check if a recipe is present in a queue
bool recipe_in_queue(RecipeQueue queue, char *name)
{
    RecipeNode *current = queue.front;
    while (current != NULL)
    {
        if (strcmp(current->recipe->name, name) == 0)
        {
            printf("ordini in sospeso\n");
            return true;
        }
        current = current->next;
    }
    return false;
}
// Time complexity: O(log n), space complexity: O(1). Finds the node with the minimum value (leftmost) in a binary search tree.
Recipe *find_minimum(Recipe *root)
{
    while (root->left != NULL)
        root = root->left;
    return root;
}
// Time complexity: O(log n), space complexity: O(log n). Recursively removes a recipe node from a binary search tree based on its name.
Recipe *rimuovi_ricetta(Recipe *root, char *name, bool *removed)
{
    if (root == NULL)
        return NULL;
    int cmp = strcmp(name, root->name);
    if (cmp < 0)
        root->left = rimuovi_ricetta(root->left, name, removed);
    else if (cmp > 0)
        root->right = rimuovi_ricetta(root->right, name, removed);
    else
    {
        if (*removed || ((order_queue.front != NULL && recipe_in_queue(order_queue, name)) || (completed_order_queue.front != NULL && recipe_in_queue(completed_order_queue, name))))
            return root;
        if (!(*removed))
        {
            printf("rimossa\n");
            *removed = true;
        }

        if (root->left == NULL)
        {
            Recipe *temp = root->right;
            free(root->ingredient_pointers);
            free(root->needed_quantities);
            free(root);
            return temp;
        }
        else if (root->right == NULL)
        {
            Recipe *temp = root->left;
            free(root->ingredient_pointers);
            free(root->needed_quantities);
            free(root);
            return temp;
        }
        Recipe *temp = find_minimum(root->right);
        // Copy data from the minimum node
        strcpy(root->name, temp->name);
        root->weight = temp->weight;
        root->num_ingredients = temp->num_ingredients;
        // Free the old pointers and allocate new memory for ingredient_pointers and needed_quantities
        free(root->ingredient_pointers);
        free(root->needed_quantities);
        root->ingredient_pointers = malloc(temp->num_ingredients * sizeof(Ingredient *));
        for (unsigned int i = 0; i < temp->num_ingredients; i++)
            root->ingredient_pointers[i] = temp->ingredient_pointers[i];
        root->needed_quantities = malloc(temp->num_ingredients * sizeof(unsigned int));
        for (unsigned int i = 0; i < temp->num_ingredients; i++)
            root->needed_quantities[i] = temp->needed_quantities[i];
        root->right = rimuovi_ricetta(root->right, temp->name, removed);
    }
    return root;
}
// Time complexity: O(log n), space complexity: O(log n). Manages the removal of a recipe from the recipes tree.
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
    bool removed = false; // To track if a recipe was removed and print only once
    recipes = rimuovi_ricetta(recipes, recipe_name, &removed);
}
// Time complexity: O(n), space complexity: O(n). Function to free the memory allocated for the ingredients
void free_queue(RecipeNode *root)
{
    while (root != NULL)
    {
        RecipeNode *temp = root;
        root = root->next;
        free(temp);
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
    RecipeNode *current = order_queue.front;
    RecipeNode *prev = NULL;
    while (current != NULL)
    {
        RecipeNode *next = current->next;
        if (can_fulfill_order(current->recipe, current->quantity))
        {
            fulfill_order(current->recipe, current->quantity);
            // Create a new node for the completed order
            RecipeNode *completed_node = (RecipeNode *)malloc(sizeof(RecipeNode));
            completed_node->recipe = current->recipe;
            completed_node->quantity = current->quantity;
            completed_node->arrival_time = current->arrival_time;
            completed_node->next = NULL;
            // Remove the node from order_queue
            if (prev == NULL)
                order_queue.front = next;
            else
                prev->next = next;
            if (current == order_queue.rear)
                order_queue.rear = prev;
            // Insert the new node into completed_order_queue in the correct position
            RecipeNode *insert_pos = find_insert_queue_position(completed_order_queue.front, completed_node);
            if (insert_pos == NULL)
            {
                completed_node->next = completed_order_queue.front;
                completed_order_queue.front = completed_node;
                if (completed_order_queue.rear == NULL)
                    completed_order_queue.rear = completed_node;
            }
            else
            {
                completed_node->next = insert_pos->next;
                insert_pos->next = completed_node;
                if (completed_node->next == NULL)
                    completed_order_queue.rear = completed_node;
            }
            // Free the memory of the removed node
            free(current);
            // Reset current to continue with the next order in the queue
            current = next;
        }
        else
        {
            prev = current;
            current = next;
        }
    }
} 
// Time complexity: O(n), space complexity: O(1). Function to find the correct insert position in the queue based on weight and arrival_time
RecipeNode *find_insert_queue_weight(RecipeNode *head, RecipeNode *new_node)
{
    RecipeNode *low = NULL;
    RecipeNode *high = head;
    while (high != NULL)
    {
        unsigned int current_weight = high->recipe->weight * high->quantity;
        unsigned int new_weight = new_node->recipe->weight * new_node->quantity;
        if (current_weight > new_weight || (current_weight == new_weight && high->arrival_time <= new_node->arrival_time))
        {
            low = high;
            high = high->next;
        }
        else
            break;
    }
    return low;
}
// Time complexity: O(n), space complexity: O(1).
void manage_courier(unsigned int courier_capacity)
{
    if (completed_order_queue.front == NULL)
    {
        printf("camioncino vuoto\n");
        return;
    }
    unsigned int current_load = 0;
    RecipeNode *prev = NULL;
    RecipeNode *current = completed_order_queue.front;
    RecipeNode *temp_courier_queue_front = NULL;
    while (current != NULL)
    {
        unsigned int order_weight = current->recipe->weight * current->quantity;
        // If adding the current order exceeds the courier capacity, break the loop
        if (current_load + order_weight > courier_capacity)
            break;
        current_load += order_weight;
        // Remove the node from completed_order_queue
        RecipeNode *next = current->next;
        if (prev == NULL)
            completed_order_queue.front = next;
        else
            prev->next = next;
        if (current == completed_order_queue.rear)
            completed_order_queue.rear = prev;
        // Insert the node into temp_courier_queue in the correct position
        RecipeNode *insert_pos = find_insert_queue_weight(temp_courier_queue_front, current);
        if (insert_pos == NULL)
        {
            current->next = temp_courier_queue_front;
            temp_courier_queue_front = current;
        }
        else
        {
            current->next = insert_pos->next;
            insert_pos->next = current;
        }
        current = next;
    }

    // Print the orders in the temporary courier queue
    RecipeNode *temp_current = temp_courier_queue_front;
    while (temp_current != NULL)
    {
        printf("%u %s %u\n", temp_current->arrival_time, temp_current->recipe->name, temp_current->quantity);
        temp_current = temp_current->next;
    }
    // Free the temporary courier queue
    free_queue(temp_courier_queue_front);
}
int main()
{
    unsigned int current_character, courier_frequency, courier_capacity;
    __attribute__((unused)) unsigned int result = fscanf(stdin, "%u %u", &courier_frequency, &courier_capacity);
    while (getchar_unlocked() != '\n')
        ;
    while ((current_character = getchar_unlocked()) != EOF)
    {
        unsigned int max_size = INITIAL_SIZE;
        char *line = malloc(INITIAL_SIZE * sizeof(char));
        unsigned int current_index = 0;
        while (current_character != '\n')
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
        if (current_index >= max_size)
        {
            char *temp = realloc(line, (max_size + 1) * sizeof(char));
            line = temp;
        }
        line[current_index] = '\0';
        switch (line[2])
        {
        case 'g':
            manage_aggiungi_ricetta(line);
            break;
        case 'm':
            manage_rimuovi_ricetta(line);
            break;
        case 'f':
            remove_spoiled_batches_from_tree(ingredients_total);
            manage_rifornimento(line);
            break;
        case 'd':
            manage_ordine(line);
            break;
        default:
            break;
        }
        free(line);
        time_elapsed++;
        if (time_elapsed % courier_frequency == 0)
            manage_courier(courier_capacity);
    }
    return 0;
}