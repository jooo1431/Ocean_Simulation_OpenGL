#pragma once

#include <filesystem>
#include <string>

namespace OGL4Core2::Core {
    class FileUtil {
    public:
        static std::filesystem::path getFullExeName();

        static std::filesystem::path findPluginResourcesPath(const std::string& path);
    };
} // namespace OGL4Core2::Core
