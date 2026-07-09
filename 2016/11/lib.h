#pragma once

#include <cstdint>
#include <string>
#include <vector>

// 2 bits for current floor
// 14 bits for items per floor (low 7 bits - chips, high 7 bits - RTGs)

// 66 665555 5555554 4444444 4433333 3333322 2222222 2111111 1111000 0000000
// 32 109876 5432109 8765432 1098765 4321098 7654321 0987654 3210987 6543210
// -------------------------------------------------------------------------
// FF 000000 3333333 3333333 2222222 2222222 1111111 1111111 0000000 0000000
// -------------------------------------------------------------------------
//    uuuuuu rrrrrrr ccccccc rrrrrrr ccccccc rrrrrrr ccccccc rrrrrrr ccccccc

// FF - current floor number
// c - chips
// r - RTGs
// u - unused

constexpr uint32_t floorCount = 4;
constexpr uint32_t floorShift = 64 - 2;
constexpr uint64_t floorMask = 3ULL << floorShift;
constexpr uint32_t maxMaterialCount = 7;
constexpr uint32_t maxItemCount = 2*maxMaterialCount;
constexpr uint32_t chipsShift = maxMaterialCount;
constexpr uint32_t chipsMask = (1 << chipsShift) - 1;
constexpr uint32_t floorItemsShift = maxItemCount;
constexpr uint64_t floorItemsMask = (1ULL << floorItemsShift) - 1ULL;
constexpr uint64_t first3FMask = floorItemsMask | (floorItemsMask << floorItemsShift) | (floorItemsMask << 2*floorItemsShift);

struct ParseResult
{
    uint64_t state;
    uint32_t materialCount;
};

ParseResult parseInput(const std::string& input);
std::vector<uint64_t> findMinStepsToGoal(uint64_t startState);
void printSteps(const std::vector<uint64_t>& steps, uint32_t materialCount);
