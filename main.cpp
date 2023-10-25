#include "include/Species.hpp"

int main()
{
    const unsigned population = 4;

    Species mySpecies(NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, population);

    for(unsigned generation = 0; true; ++generation)
    {
        mySpecies.play_one_generation();

        std::cout<<"\ngeneration: "<< generation<< '\n';
        mySpecies.print_best_scorer();

        mySpecies.repolulate();
    }
    return 0;
}