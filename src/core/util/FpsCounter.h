#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace OGL4Core2::Core {
    class FpsCounter {
    public:
        explicit FpsCounter(double updateFrequency = 1.0, std::size_t bufferSize = 30);
        ~FpsCounter() = default;

        bool tick();

        double getFps();

        std::string getFpsString();

    private:
        double updateFrequency;
        std::size_t bufferSize;
        std::chrono::high_resolution_clock::time_point lastUpdate;
        std::size_t currentPos;
        std::vector<std::chrono::high_resolution_clock::time_point> timestamps;
    };
} // namespace OGL4Core2::Core
