#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

struct Ingredient
{
    int capacity;
    int durability;
    int flavor;
    int texture;
    int calories;
};

static int power(int x, int p)
{
    int result = 1;
    for (int i = 0; i < p; ++i)
    {
        result *= x;
    }
    return result;
}

struct Recipe
{
    std::vector<int> spoons;
    int product;
};

Recipe findBestRecipe(const std::vector<Ingredient>& ingredients, int totalTeaspoons, bool withCalories = false)
{
    const int ingredientsCount{ static_cast<int>(ingredients.size()) };
    std::vector<int> recipe(ingredientsCount);
    std::vector<int> spoons(ingredientsCount);
    int totalIndices = power(totalTeaspoons, ingredientsCount);
    int maxProduct = -1;
    for (int i = 0; i < totalIndices; ++i)
    {
        int sum = 0;
        int foo = i;
        for (int j = 0; j < ingredientsCount; ++j)
        {
            int x = foo % totalTeaspoons;
            sum += x;
            spoons[j] = x;
            foo /= totalTeaspoons;
        }

        if (sum == totalTeaspoons)
        {
            int capacity = 0;
            for (int j = 0; j < ingredientsCount; ++j) capacity += ingredients[j].capacity * spoons[j];
            if (capacity < 0) capacity = 0;

            int durability = 0;
            for (int j = 0; j < ingredientsCount; ++j) durability += ingredients[j].durability * spoons[j];
            if (durability < 0) durability = 0;

            int flavor = 0;
            for (int j = 0; j < ingredientsCount; ++j) flavor += ingredients[j].flavor * spoons[j];
            if (flavor < 0) flavor = 0;

            int texture = 0;
            for (int j = 0; j < ingredientsCount; ++j) texture += ingredients[j].texture * spoons[j];
            if (texture < 0) texture = 0;

            int calories = 0;
            for (int j = 0; j < ingredientsCount; ++j) calories += ingredients[j].calories * spoons[j];
            if (calories < 0) calories = 0;

            if (withCalories && calories != 500)
            {
                continue;
            }

            int product = capacity * durability * flavor * texture;
            if (product > maxProduct)
            {
                maxProduct = product;
                for (int j = 0; j < ingredientsCount; ++j)
                {
                    recipe[j] = spoons[j];
                }
            }
        }
    }
    return { recipe, maxProduct };
}

static std::vector<Ingredient> parseInput(std::basic_istream<char>& input)
{
    std::vector<Ingredient> ingredients;
    std::string dummy;

    while (true)
    {
        Ingredient ingredient;

        input >> dummy >> dummy;
        if (!input)
        {
            break;
        }

        input >> ingredient.capacity >> dummy >> dummy;
        input >> ingredient.durability >> dummy >> dummy;
        input >> ingredient.flavor >> dummy >> dummy;
        input >> ingredient.texture >> dummy >> dummy;
        input >> ingredient.calories;

        assert(input);

        ingredients.push_back(ingredient);
    }
    return ingredients;
}

int main()
{
    std::istringstream exampleInputStream{ R"(
Butterscotch: capacity -1, durability -2, flavor 6, texture 3, calories 8
Cinnamon: capacity 2, durability 3, flavor -2, texture -1, calories 3
)" };
    auto exampleIngredients{ parseInput(exampleInputStream) };
    auto exampleRecipe = findBestRecipe(exampleIngredients, 100);
    assert(exampleRecipe.spoons[0] == 44);
    assert(exampleRecipe.spoons[1] == 56);
    assert(exampleRecipe.product == 62842880);

    std::ifstream inputStream{ "input.txt" };
    assert(inputStream);
    auto ingredients{ parseInput(inputStream) };

    auto recipe{ findBestRecipe(ingredients, 100) };
    std::cout << recipe.product << '\n';

    auto recipe2{ findBestRecipe(ingredients, 100, true) };
    std::cout << recipe2.product << '\n';

    return 0;
}