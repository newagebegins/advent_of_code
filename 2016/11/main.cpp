#include "lib.h"

#include <iostream>
#include <string>

static void findSolution(const std::string& input, bool shouldPrintSteps = false)
{
    ParseResult pr = parseInput(input);
    std::vector<uint64_t> steps = findMinStepsToGoal(pr.state);
    if (shouldPrintSteps)
        printSteps(steps, pr.materialCount);
    std::cout << steps.size() - 1 << '\n';
}

int main()
{
    findSolution(R"(The first floor contains a hydrogen-compatible microchip and a lithium-compatible microchip.
The second floor contains a hydrogen generator.
The third floor contains a lithium generator.
The fourth floor contains nothing relevant.)");
    findSolution(R"(The first floor contains a thulium generator, a thulium-compatible microchip, a plutonium generator, and a strontium generator.
The second floor contains a plutonium-compatible microchip and a strontium-compatible microchip.
The third floor contains a promethium generator, a promethium-compatible microchip, a ruthenium generator, and a ruthenium-compatible microchip.
The fourth floor contains nothing relevant.)");
    findSolution(R"(The first floor contains a thulium generator, a thulium-compatible microchip, a plutonium generator, a strontium generator, a elerium generator, a elerium-compatible microchip, a dilithium generator, a dilithium-compatible microchip.
The second floor contains a plutonium-compatible microchip and a strontium-compatible microchip.
The third floor contains a promethium generator, a promethium-compatible microchip, a ruthenium generator, and a ruthenium-compatible microchip.
The fourth floor contains nothing relevant.)");

    return 0;
}
