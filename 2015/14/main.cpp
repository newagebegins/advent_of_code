#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

struct Reindeer
{
    int flySpeed;
    int flyDuration;
    int restDuration;
};

static int getDistanceTraveled(const Reindeer& reindeer, int raceDuration)
{
    int distanceTraveled = 0;
    int time = 0;
    bool flying = true;

    while (time != raceDuration)
    {
        if (flying)
        {
            int step = reindeer.flyDuration;
            if (time + step > raceDuration)
            {
                step = raceDuration - time;
            }
            time += step;
            distanceTraveled += reindeer.flySpeed * step;
            flying = false;
        }
        else
        {
            int step = reindeer.restDuration;
            if (time + step > raceDuration)
            {
                step = raceDuration - time;
            }
            time += step;
            flying = true;
        }
    }

    return distanceTraveled;
}

static int getWinningDistance(const std::vector<Reindeer>& reindeers, int raceDuration)
{
    std::vector<int> distances;
    std::ranges::transform(reindeers, std::back_inserter(distances), [raceDuration](const Reindeer& reindeer) {
        return getDistanceTraveled(reindeer, raceDuration);
    });
    return std::ranges::max(distances);
}

static std::vector<Reindeer> parseInput(std::basic_istream<char>&& input)
{
    std::vector<Reindeer> reindeers;
    std::string dummy;

    while (true)
    {
        Reindeer r;
        input >> dummy >> dummy >> dummy;
        if (!input)
        {
            break;
        }
        input >> r.flySpeed;
        input >> dummy >> dummy;
        input >> r.flyDuration;
        input >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy;
        input >> r.restDuration;
        input >> dummy;
        reindeers.push_back(r);
    }

    return reindeers;
}

struct ReindeerState
{
    bool flying{ true };
    int distanceTraveled{ 0 };
    int time{ 0 };
    int points{ 0 };
};

int getWinningPoints(const std::vector<Reindeer>& reindeers, int raceDuration)
{
    std::vector<ReindeerState> state(reindeers.size());

    for (int time = 0; time < raceDuration; ++time)
    {
        // simulate
        for (int i = 0; i < reindeers.size(); ++i)
        {
            if (state[i].flying)
            {
                state[i].distanceTraveled += reindeers[i].flySpeed;
                ++(state[i].time);
                if (state[i].time == reindeers[i].flyDuration)
                {
                    state[i].time = 0;
                    state[i].flying = false;
                }
            }
            else
            {
                ++(state[i].time);
                if (state[i].time == reindeers[i].restDuration)
                {
                    state[i].time = 0;
                    state[i].flying = true;
                }
            }
        }

        // award points
        int maxDistance = 0;
        for (int i = 0; i < reindeers.size(); ++i)
        {
            if (state[i].distanceTraveled > maxDistance)
            {
                maxDistance = state[i].distanceTraveled;
            }
        }

        for (int i = 0; i < reindeers.size(); ++i)
        {
            if (state[i].distanceTraveled == maxDistance)
            {
                state[i].points++;
            }
        }
    }

    int maxPoints = 0;
    for (int i = 0; i < reindeers.size(); ++i)
    {
        if (state[i].points > maxPoints)
        {
            maxPoints = state[i].points;
        }
    }

    return maxPoints;
}

int main()
{
    const std::vector<Reindeer> exampleReindeers{ parseInput(std::istringstream { R"(


Comet can fly 14 km/s for 10 seconds, but then must rest for 127 seconds.
Dancer can fly 16 km/s for 11 seconds, but then must rest for 162 seconds.


)" })};
    std::cout << getWinningDistance(exampleReindeers, 1000) << '\n';
    std::cout << getWinningPoints(exampleReindeers, 1000) << '\n';

    const std::vector<Reindeer> reindeers{ parseInput(std::ifstream { "input.txt" }) };
    std::cout << getWinningDistance(reindeers, 2503) << '\n';
    std::cout << getWinningPoints(reindeers, 2503) << '\n';

    return 0;
}
