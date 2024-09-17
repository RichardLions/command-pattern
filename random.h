#pragma once

#include <random>

namespace Random
{
    namespace
    {
        using RandomGenerator = std::mt19937;

        static RandomGenerator& GetRandomGenerator()
        {
            static std::random_device device{};
            static RandomGenerator generator{device()}; // Seed
            return generator;
        }
    }

    /// Returns true/false
    static bool RandomBool()
    {
        constexpr double_t probability{0.5};
        static std::bernoulli_distribution distribution{probability};
        return distribution(GetRandomGenerator());
    }

    /// Returns a value between and including min~max
    static int32_t RandomInRange(const int32_t min, const int32_t max)
    {
        std::uniform_int_distribution<> distribution(min, max);
        return distribution(GetRandomGenerator());
    }
}
