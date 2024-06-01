#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define WORD_SIZE 256    // Size of a single word
#define INITIAL_SIZE 768 // Initial size of the line buffer (WORD_SIZE * 3)

unsigned int courier_frequency, courier_capacity;
unsigned int num_recipes = 0;
unsigned int max_recipes = 0;
unsigned int num_ingredients_total = 0;
unsigned int max_ingredients_total = 0;

typedef struct
{
    char name[WORD_SIZE];
    unsigned int quantity;
} Ingredient;

Ingredient *ingredients_total = NULL;

typedef struct
{
    char name[WORD_SIZE];
    Ingredient *ingredients;
    unsigned int num_ingredients;
} Recipe;

Recipe *recipes = NULL;

unsigned int find_recipe_index(char *name)
{
    for (unsigned int i = 0; i < num_recipes; i++)
        if (strcmp(recipes[i].name, name) == 0)
            return i;
    return -1;
}

unsigned int find_ingredient_index(char *name)
{
    for (unsigned int i = 0; i < num_ingredients_total; i++)
        if (strcmp(ingredients_total[i].name, name) == 0)
            return i;
    return -1;
}

void init_recipe(Recipe *recipe, char *name, unsigned int num_ingredients)
{
    strcpy(recipe->name, name);
    recipe->ingredients = malloc(num_ingredients * sizeof(Ingredient));
    recipe->num_ingredients = num_ingredients;
}

void add_ingredient(char *name, unsigned int quantity)
{
    // Check if the ingredient already exists
    int index = find_ingredient_index(name);
    if (index != -1)
        ingredients_total[index].quantity += quantity;
    else
    {
        // Increase the total ingredients array size if necessary
        if (num_ingredients_total >= max_ingredients_total)
        {
            max_ingredients_total++;
            ingredients_total = realloc(ingredients_total, max_ingredients_total * sizeof(Ingredient));
        }
        // Add the new ingredient
        strcpy(ingredients_total[num_ingredients_total].name, name);
        ingredients_total[num_ingredients_total].quantity = quantity;
        num_ingredients_total++;
    }
}

void aggiungi_ricetta(char *name, char **ingredients, unsigned int *quantities, unsigned int num_ingredients, unsigned int max_ingredients)
{
    // Check if recipe already exists else it increases recipe array size if necessary
    if (find_recipe_index(name) != -1)
    {
        printf("ignorato\n");
        return;
    }
    else
    {
        max_recipes++;
        recipes = realloc(recipes, max_recipes * sizeof(Recipe));
    }

    // Create new recipe
    init_recipe(&recipes[num_recipes], name, num_ingredients);
    for (unsigned int i = 0; i < num_ingredients; i++)
    {
        add_ingredient(ingredients[i], quantities[i]);
        strcpy(recipes[num_recipes].ingredients[i].name, ingredients[i]);
        recipes[num_recipes].ingredients[i].quantity = quantities[i];
    }

    // Increment number of recipes
    num_recipes++;

    printf("aggiunta\n");
}

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
    while (token != NULL)
    {
        if (num_ingredients < max_ingredients)
        {
            ingredients[num_ingredients] = strdup(token);
            num_ingredients++;
        }
        else
            break;
        token = strtok(NULL, " ");
        if (token != NULL)
        {
            quantities[num_ingredients - 1] = atoi(token);
            token = strtok(NULL, " ");
        }
        else
        {
            // TODO ? if last doesen't have quantity
        }
    }

    // Call the aggiungi_ricetta function
    aggiungi_ricetta(name, ingredients, quantities, num_ingredients, max_ingredients);

    // Free the memory allocated for the ingredients
    for (unsigned int i = 0; i < num_ingredients; i++)
        free(ingredients[i]);
    free(ingredients);
    free(quantities);
}
int main()
{
    unsigned int current_character;
    // Read the two numbers from the first line
    scanf("%u %u", &courier_frequency, &courier_capacity);
    // Read characters until EOF
    while ((current_character = getchar()) != EOF)
    {
        // Check if the current character is a digit (?)
        if (isdigit(current_character))
            continue; // TODO check, probably will delete

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
                max_size += WORD_SIZE;                               // Increment the size by WORD_SIZE
                char *temp = realloc(line, max_size * sizeof(char)); // Memory reallocation
                if (temp == NULL)
                line = temp;
            }
            line[current_index++] = current_character; // Add the character to the array
            current_character = getchar();             // Read the next character
        }

        line[current_index] = '\0'; // Terminate the string with '\0'

        // Process the command based on the third character of the line
        if (current_index >= 3)
            switch (line[2])
            {
            case 'g':
                manage_aggiungi_ricetta(line);
            case 'm':
                // rimuovi_ricetta
                break;
            case 'f':
                // rifornimento
                break;
            case 'd':
                // ordine
                break;
            default:
                break;
            }

        free(line);
    }

    return 0;
}
