#include "lib.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

static uint64_t setFloorItem(uint64_t srcState, uint32_t floor, uint32_t materialIndex, bool isMicrochip)
{
    uint64_t state = srcState;
    uint32_t itemShift = floor*floorItemsShift + materialIndex + (isMicrochip ? 0 : maxMaterialCount);
    uint64_t itemBit = (1ULL << itemShift);
    assert((state & itemBit) == 0);
    state |= itemBit;
    return state;
}

ParseResult parseInput(const std::string& input)
{
    uint64_t state = 0;
    std::stringstream ss(input);
    std::string line;
    std::unordered_map<std::string, uint32_t> floorFromString =
    {
        {"first",  0},
        {"second", 1},
        {"third",  2},
        {"fourth", 3},
    };
    uint32_t materialCount = 0;
    std::unordered_map<std::string, uint32_t> materialIndexFromName;
    while (std::getline(ss, line))
    {
        std::stringstream lineStream(line);
        std::string word;
        lineStream >> word;
        assert(word == "The");
        lineStream >> word;
        uint32_t floor = floorFromString.at(word);
        lineStream >> word;
        assert(word == "floor");
        lineStream >> word;
        assert(word == "contains");
        while (lineStream >> word)
        {
            if (word == "a")
            {
                lineStream >> word;
                bool isMicrochip = false;
                std::string::size_type hyphenPos = word.find('-');
                if (hyphenPos != std::string::npos)
                {
                    word.erase(hyphenPos);
                    isMicrochip = true;
                }
                uint32_t materialIndex;
                auto it = materialIndexFromName.find(word);
                if (it == materialIndexFromName.end())
                {
                    materialIndex = materialCount++;
                    materialIndexFromName[word] = materialIndex;
                }
                else
                {
                    materialIndex = it->second;
                }
                state = setFloorItem(state, floor, materialIndex, isMicrochip);
            }
        }
    }
    
    return { state, materialCount };
}

static bool chipsAreSafe(uint32_t floorItems)
{
    bool result = true;
    uint32_t chips = floorItems & chipsMask;
    uint32_t rtgs = floorItems >> chipsShift;

    // If there are no chips or no RTGs then it is safe
    if (chips && rtgs)
    {
        for (int material = 0; material < maxMaterialCount; ++material)
        {
            int matMask = 1 << material;
            if (chips & matMask)
            {
                if (rtgs & matMask)
                {
                    // There is a compatible RTG for this chip, so it is safe
                }
                else
                {
                    // There is no compatible RTG, but there is uncompatible one, so the chip will fry
                    result = false;
                    break;
                }
            }
        }
    }

    return result;
}

static uint32_t getFloor(uint64_t state)
{
    return state >> floorShift;
}

static uint64_t setFloor(uint64_t srcState, uint32_t floor)
{
    uint64_t state = srcState;
    assert(floor < floorCount);
    state &= (~floorMask);
    state |= (static_cast<uint64_t>(floor) << floorShift);
    return state;
}

static uint32_t getFloorItems(uint64_t state, uint32_t floor)
{
    assert(floor < floorCount);
    return (state >> (floor*floorItemsShift)) & floorItemsMask;
}

static bool isGoalState(uint64_t state)
{
    return (state & first3FMask) == 0;
}

static uint64_t setFloorItems(uint64_t& srcState, uint32_t floor, uint32_t newItems)
{
    uint64_t state = srcState;
    state &= ~(floorItemsMask << (floor*floorItemsShift));
    state |= (static_cast<uint64_t>(newItems) << (floor*floorItemsShift));
    return state;
}

static uint64_t createNewState(uint64_t srcState, uint32_t curFloor, uint32_t newCurItems, uint32_t tgtFloor, uint32_t newTgtItems)
{
    uint64_t result = srcState;
    result = setFloor(result, tgtFloor);
    result = setFloorItems(result, curFloor, newCurItems);
    result = setFloorItems(result, tgtFloor, newTgtItems);
    return result;
}

static std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
createStateTree(uint64_t startState)
{
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> stateTree;
    std::unordered_set<uint64_t> stateSet = { startState };
    std::queue<uint64_t> stateQueue;
    stateQueue.push(startState);

    while (!stateQueue.empty())
    {
        uint64_t state = stateQueue.front();
        stateQueue.pop();
        uint32_t curFloor = getFloor(state);
        uint32_t items = getFloorItems(state, curFloor);
        int floorDeltas[2];
        int floorDeltaCount = 0;
        if (curFloor == 0)
        {
            floorDeltas[0] = 1;
            floorDeltaCount = 1;
        }
        else if (curFloor == floorCount-1)
        {
            floorDeltas[0] = -1;
            floorDeltaCount = 1;
        }
        else
        {
            assert(curFloor < floorCount);
            floorDeltas[0] = 1;
            floorDeltas[1] = -1;
            floorDeltaCount = 2;
        }
        assert((items & (1 << maxItemCount)) == 0);
        for (int floorDeltaIndex = 0; floorDeltaIndex < floorDeltaCount; ++floorDeltaIndex)
        {
            int floorDelta = floorDeltas[floorDeltaIndex];
            uint32_t tgtFloor = curFloor + floorDelta;
            assert(tgtFloor < floorCount);
            for (uint32_t itemIndex1 = 0; itemIndex1 < maxItemCount; ++itemIndex1)
            {
                uint32_t item1 = items & (1 << itemIndex1);
                if (item1)
                {
                    for (uint32_t itemIndex2 = itemIndex1 + 1; itemIndex2 <= maxItemCount; ++itemIndex2)
                    {
                        uint32_t item2 = items & (1 << itemIndex2);
                        if (item2 || (itemIndex2 == maxItemCount))
                        {
                            uint32_t movedItems = item1 | item2;
                            assert(movedItems);
                            uint32_t newCurItems = items & ~movedItems;
                            uint32_t newTgtItems = getFloorItems(state, tgtFloor);
                            assert((newTgtItems & movedItems) == 0);
                            newTgtItems |= movedItems;
                            if (chipsAreSafe(newCurItems) && chipsAreSafe(newTgtItems))
                            {
                                uint64_t newState = createNewState(state, curFloor, newCurItems, tgtFloor, newTgtItems);
                                auto [insertIter, inserted] = stateSet.insert(newState);
                                if (inserted)
                                {
                                    stateTree[state].insert(newState);
                                    stateQueue.push(newState);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return stateTree;
}

// Build a tree of all possible moves
// Use breadth-first search to find the shortest path to the goal state
std::vector<uint64_t> findMinStepsToGoal(uint64_t startState)
{
    std::vector<uint64_t> steps;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> stateTree = createStateTree(startState);
    std::queue<uint64_t> stateQueue;
    std::unordered_map<uint64_t, uint64_t> stateToParentMap;
    stateQueue.push(startState);
    stateToParentMap.insert({startState, 0});
    uint64_t goalState = 0;
    while (!stateQueue.empty())
    {
        uint64_t state = stateQueue.front();
        stateQueue.pop();
        if (isGoalState(state))
        {
            goalState = state;
            break;
        }
        for (uint64_t s : stateTree[state])
        {
            stateQueue.push(s);
            assert(stateToParentMap.find(s) == stateToParentMap.end());
            stateToParentMap.insert({s, state});
        }
    }
    assert(goalState);
    for (uint64_t state = goalState; state; state = stateToParentMap.at(state))
    {
        steps.push_back(state);
    }
    std::reverse(steps.begin(), steps.end());
    return steps;
}

static void printState(uint64_t state, uint32_t materialCount)
{
    uint32_t elevatorFloor = getFloor(state);
    constexpr std::array<std::pair<char, uint32_t>, 2> itemTypeInfo =
    {
        std::make_pair('M', 0),
        std::make_pair('G', maxMaterialCount)
    };
    for (int floor = 3; floor >= 0; --floor)
    {
        std::cout << 'F' << (floor+1) << ' ';
        std::cout << ((static_cast<uint32_t>(floor) == elevatorFloor) ? 'E' : '.') << ' ';
        uint32_t items = getFloorItems(state, floor);
        for (auto [letter, offset] : itemTypeInfo)
        {
            for (uint32_t itemIndex = 0; itemIndex < materialCount; ++itemIndex)
            {
                if (items & (1 << (itemIndex + offset)))
                {
                    std::cout << itemIndex << letter;
                }
                else
                {
                    std::cout << ". ";
                }
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
}

void printSteps(const std::vector<uint64_t>& steps, uint32_t materialCount)
{
    for (uint32_t step = 0; step < steps.size(); ++step)
    {
        std::cout << "Step " << step << ":\n";
        printState(steps[step], materialCount);
        std::cout << '\n';
    }
}
