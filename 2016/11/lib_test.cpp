#include "lib.h"

#include <gtest/gtest.h>

TEST(parseInput, input1)
{
    std::string input = R"(The first floor contains a hydrogen-compatible microchip and a lithium-compatible microchip.
The second floor contains a hydrogen generator.
The third floor contains a lithium generator.
The fourth floor contains nothing relevant.)";
    ParseResult pr = parseInput(input);
    // hydrogen -> 0
    // lithium  -> 1
    uint64_t expectedState = 0b00000000'00000000000000'00000100000000'00000010000000'00000000000011ULL;
    EXPECT_EQ(pr.state, expectedState);
}

TEST(parseInput, input2)
{
    std::string input = R"(The first floor contains a thulium generator, a thulium-compatible microchip, a plutonium generator, and a strontium generator.
The second floor contains a plutonium-compatible microchip and a strontium-compatible microchip.
The third floor contains a promethium generator, a promethium-compatible microchip, a ruthenium generator, and a ruthenium-compatible microchip.
The fourth floor contains nothing relevant.)";
    ParseResult pr = parseInput(input);
    // thulium    -> 0
    // plutonium  -> 1
    // strontium  -> 2
    // promethium -> 3
    // ruthenium  -> 4
    uint64_t expectedState = 0b00000000'00000000000000'00110000011000'00000000000110'00001110000001ULL;
    EXPECT_EQ(pr.state, expectedState);
}

TEST(findMinStepsToGoal, input1)
{
    uint64_t state = 0b00000000'00000000000000'00000000000000'00000000000000'00000000000001ULL;
    std::vector<uint64_t> steps = findMinStepsToGoal(state);
    EXPECT_EQ(steps.size(), 4);
    EXPECT_EQ(steps[0],  0b00000000'00000000000000'00000000000000'00000000000000'00000000000001ULL);
    EXPECT_EQ(steps[1],  0b01000000'00000000000000'00000000000000'00000000000001'00000000000000ULL);
    EXPECT_EQ(steps[2],  0b10000000'00000000000000'00000000000001'00000000000000'00000000000000ULL);
    EXPECT_EQ(steps[3],  0b11000000'00000000000001'00000000000000'00000000000000'00000000000000ULL);
}

TEST(findMinStepsToGoal, testInput)
{
    // hydrogen -> 0
    // lithium  -> 1
    uint64_t state = 0b00000000'00000000000000'00000100000000'00000010000000'00000000000011ULL;
    std::vector<uint64_t> steps = findMinStepsToGoal(state);
    EXPECT_EQ(steps.size(), 12);
    /*
    EXPECT_EQ(steps[0],  0b00000000'00000000000000'00000100000000'00000010000000'00000000000011ULL);
    EXPECT_EQ(steps[1],  0b01000000'00000000000000'00000100000000'00000010000001'00000000000010ULL);
    EXPECT_EQ(steps[2],  0b10000000'00000000000000'00000110000001'00000000000000'00000000000010ULL);
    EXPECT_EQ(steps[3],  0b01000000'00000000000000'00000110000000'00000000000001'00000000000010ULL);
    EXPECT_EQ(steps[4],  0b00000000'00000000000000'00000110000000'00000000000000'00000000000011ULL);
    EXPECT_EQ(steps[5],  0b01000000'00000000000000'00000110000000'00000000000011'00000000000000ULL);
    EXPECT_EQ(steps[6],  0b10000000'00000000000000'00000110000011'00000000000000'00000000000000ULL);
    EXPECT_EQ(steps[7],  0b11000000'00000000000011'00000110000000'00000000000000'00000000000000ULL);
    EXPECT_EQ(steps[8],  0b10000000'00000000000010'00000110000001'00000000000000'00000000000000ULL);
    EXPECT_EQ(steps[9],  0b11000000'00000110000010'00000000000001'00000000000000'00000000000000ULL);
    EXPECT_EQ(steps[10], 0b10000000'00000110000000'00000000000011'00000000000000'00000000000000ULL);
    EXPECT_EQ(steps[11], 0b11000000'00000110000011'00000000000000'00000000000000'00000000000000ULL);
    */
}
