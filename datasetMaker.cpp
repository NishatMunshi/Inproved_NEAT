#include <fstream>
#include "libraries/randomNumberGenerator.hpp"

int main()
{

    std::ofstream file("xor.txt");

    for (unsigned i = 0; i < 500; ++i)
    {
        int randomX = random_bool.generate();
        int randomY = random_bool.generate();
        int randomZ = random_bool.generate();

        auto result = ((randomX or randomY) and randomZ) ? 1 : -1;
        randomX = randomX ? 1 : -1;
        randomY = randomY ? 1 : -1;
        randomZ = randomZ ? 1 : -1;
        file << randomX << ' ' << randomY << ' ' << randomZ << ' ' << result << '\n';
    }

    file.close();
    return 0;
}