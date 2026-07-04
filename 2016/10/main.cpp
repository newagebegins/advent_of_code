#include <array>
#include <cassert>
#include <iostream>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using Value = int;
using BotId = int;
using OutputBinId = int;
using ValuePair = std::array<Value, 2>;

struct Bot
{
	ValuePair values = {};
	int valueCount = 0;

	void receiveValue(Value v)
	{
		if (valueCount == 0)
		{
			values[0] = v;
			valueCount = 1;
		}
		else if (valueCount == 1)
		{
			if (values[0] < v)
			{
				values[1] = v;
			}
			else
			{
				values[1] = values[0];
				values[0] = v;
			}
			valueCount = 2;
		}
		else
		{
			assert(!"invalid valueCount");
		}
	}
};

enum TargetType
{
	TargetType_Output,
	TargetType_Bot,
};

struct Instruction
{
	TargetType targetType;
	int targetId;
};

using Bots = std::unordered_map<BotId, Bot>;
using InstructionPair = std::array<Instruction, 2>;
using Instructions = std::unordered_map<BotId, InstructionPair>;
using OutputBin = std::unordered_set<Value>;

struct SimulationResult
{
	std::unordered_map<BotId, ValuePair> comparedValues;
	std::unordered_map<OutputBinId, OutputBin> outputBins;
};

static SimulationResult simulate(Bots& bots, const Instructions& instructions)
{
	assert(!bots.empty());

	SimulationResult result;

	while (true)
	{
		BotId botId = 0;
		Bot* bot = 0;
		for (auto it = bots.begin(); it != bots.end(); ++it)
		{
			Bot *b = &it->second;
			if (b->valueCount == 2)
			{
				botId = it->first;
				bot = b;
				break;
			}
		}
		if (!bot)
		{
			break;
		}
		auto it = instructions.find(botId);
		assert(it != instructions.end());

		result.comparedValues[botId] = bot->values;
		InstructionPair instPair = it->second;

		for (int valueIndex = 0; valueIndex < 2; ++valueIndex)
		{
			Value value = bot->values[valueIndex];
			Instruction inst = instPair[valueIndex];
			switch (inst.targetType)
			{
			case TargetType_Output:
				result.outputBins[inst.targetId].insert(value);
				break;
			case TargetType_Bot:
			{
				Bot& targetBot = bots[inst.targetId];
				targetBot.receiveValue(value);
			}
				break;
			default:
				assert(!"invalid target");
				break;
			}
		}

		bot->valueCount = 0;
	}

	return result;
}

std::pair<Bots, Instructions> processInput(std::istream& inputStream)
{
	Bots bots;
	Instructions instructions;

	std::string line;
	while (std::getline(inputStream, line))
	{
		std::stringstream ss(line);
		std::string word;
		ss >> word;
		if (word == "value")
		{
			// value 61 goes to bot 49
			Value v;
			ss >> v;
			ss >> word;
			assert(word == "goes");
			ss >> word;
			assert(word == "to");
			ss >> word;
			assert(word == "bot");
			BotId botId;
			ss >> botId;
			bots[botId].receiveValue(v);
		}
		else
		{
			assert(word == "bot");
			// bot 28 gives low to output 0 and high to bot 61
			// bot 19 gives low to bot 77 and high to bot 181
			BotId botId;
			ss >> botId;
			InstructionPair instPair;
			ss >> word;
			assert(word == "gives");
			ss >> word;
			assert(word == "low");
			ss >> word;
			assert(word == "to");
			ss >> word;
			if (word == "output")
			{
				instPair[0].targetType = TargetType_Output;
			}
			else
			{
				assert(word == "bot");
				instPair[0].targetType = TargetType_Bot;
			}
			ss >> instPair[0].targetId;
			ss >> word;
			assert(word == "and");
			ss >> word;
			assert(word == "high");
			ss >> word;
			assert(word == "to");
			ss >> word;
			if (word == "output")
			{
				instPair[1].targetType = TargetType_Output;
			}
			else
			{
				assert(word == "bot");
				instPair[1].targetType = TargetType_Bot;
			}
			ss >> instPair[1].targetId;
			instructions[botId] = instPair;
		}
	}

	return std::make_pair(bots, instructions);
}

int main()
{
	std::ifstream file("input.txt");

	const char* testInput = R"(value 5 goes to bot 2
bot 2 gives low to bot 1 and high to bot 0
value 3 goes to bot 1
bot 1 gives low to output 1 and high to bot 0
bot 0 gives low to output 2 and high to output 0
value 2 goes to bot 2)";

	std::istringstream iss(testInput);
	std::pair<Bots, Instructions> testBi = processInput(iss);
	SimulationResult simRes1 = simulate(testBi.first, testBi.second);

	std::pair<Bots, Instructions> bi = processInput(file);
	SimulationResult simRes = simulate(bi.first, bi.second);

	for (const auto& it : simRes.comparedValues)
	{
		ValuePair vp = it.second;
		if (vp[0] == 17 && vp[1] == 61)
		{
			std::cout << it.first << '\n';
			break;
		}
	}

	int product = 1;
	for (OutputBinId binId = 0; binId <= 2; ++binId)
	{
		for (auto v : simRes.outputBins[binId])
		{
			product *= v;
		}
	}
	std::cout << product << '\n';

	return 0;
}
