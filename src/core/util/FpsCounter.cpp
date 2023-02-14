#include "FpsCounter.h"

#include <iomanip>
#include <sstream>

using namespace OGL4Core2::Core;

FpsCounter::FpsCounter(double updateFrequency, std::size_t bufferSize)
    : updateFrequency(updateFrequency),
      bufferSize(bufferSize),
      currentPos(bufferSize - 1) {
    auto now = std::chrono::high_resolution_clock::now();
    lastUpdate = now;
    timestamps = std::vector<std::chrono::high_resolution_clock::time_point>(bufferSize, now);
}

bool FpsCounter::tick() {
    auto now = std::chrono::high_resolution_clock::now();
    currentPos = (currentPos + 1) % bufferSize;
    timestamps[currentPos] = now;
    return std::chrono::duration<double>(now - lastUpdate).count() >= updateFrequency;
}

double FpsCounter::getFps() {
    lastUpdate = std::chrono::high_resolution_clock::now();
    auto time = timestamps[currentPos] - timestamps[(currentPos + 1) % bufferSize];
    return static_cast<double>(bufferSize - 1) / std::chrono::duration<double>(time).count();
}

std::string FpsCounter::getFpsString() {
    const auto fps = getFps();
    const double ms = 1000.0 / fps;
    std::stringstream s;
    s << std::fixed << std::setprecision(2) << fps << " FPS / " << ms << " ms";
    return s.str();
}
