#include "FileUtil.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include <windows.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#include <libproc.h>
#endif

using namespace OGL4Core2::Core;

std::filesystem::path FileUtil::getFullExeName() {
#ifdef WIN32
    std::vector<wchar_t> filename;
    DWORD length;
    do {
        filename.resize(filename.size() + 1024);
        length = GetModuleFileNameW(NULL, filename.data(), static_cast<DWORD>(filename.size()));
    } while (length >= filename.size());
    filename.resize(length);
    return std::filesystem::path(std::wstring(filename.begin(), filename.end()));
#elif __APPLE__
    char path_buf[PROC_PIDPATHINFO_MAXSIZE];

    int len = proc_pidpath(getpid(), path_buf,
PROC_PIDPATHINFO_MAXSIZE);
    if (len <= 0) {
        printf("got error %d reading pid path\n", len);
        throw std::runtime_error("Cannot read process name!");
    }
    std::filesystem::path p(path_buf);
    if (std::filesystem::is_symlink(p)) {
        p = std::filesystem::read_symlink(p);
    }
    if (!std::filesystem::exists(p)) {
        throw std::runtime_error("Cannot read process name!");
    }
    return p;

#else
    std::filesystem::path p("/proc/self/exe");
    if (!std::filesystem::exists(p) || !std::filesystem::is_symlink(p)) {
        throw std::runtime_error("Cannot read process name!");
    }
    return std::filesystem::read_symlink(p);
#endif
}

std::filesystem::path FileUtil::findPluginResourcesPath(const std::string& path) {
    // Idea:
    // We need to distinguish between running the program from cmake build directory and running from installed
    // directory. When running from installed directory we want to use the installed resources, which are located in a
    // well-defined directory relative to the program binary. When running from cmake build directory we want the
    // resources from the source directory of the plugin. In addition, the path names are different in this two cases.
    // The installed directory structure looks like this:
    //   install_dir/bin/OGL4Core2
    //   install_dir/resources/PluginName/shader.glsl
    // Therefore we can use the relative path "../resources/PluginName", when we know the full path of the binary. In
    // the source directory the structure looks like this:
    //   source_dir/src/plugins/PluginName/Resources/shader.glsl
    // There is an extra "Resources" directory after the plugin name. And as the cmake build directory can be at any
    // location, we do not know where source_dir is. We write a config file to cmake_build_dir directly next to the
    // binary with the path to the "source_dir/src/plugins" dir.
    // Also, we can check for existence of this config file to distinguish between the two cases.

    std::filesystem::path fullExeName = FileUtil::getFullExeName();
    // Remove extension (on Windows).
    std::filesystem::path configFilePath = (fullExeName.parent_path() / fullExeName.stem()).string() + ".config";
    // Visual Studio creates "Release" or "Debug" directories for binary. Check parent directory for config.
    if (!std::filesystem::exists(configFilePath)) {
        configFilePath = (fullExeName.parent_path().parent_path() / fullExeName.stem()).string() + ".config";
    }
    if (std::filesystem::exists(configFilePath)) {
        if (!std::filesystem::is_regular_file(configFilePath)) {
            throw std::runtime_error("Cannot read config file!");
        }
        // running from build directory
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            throw std::runtime_error("Cannot read config file!");
        }
        std::string line;
        getline(configFile, line);
        if (line.rfind("PLUGINS_SOURCE_DIR=", 0) != 0) {
            throw std::runtime_error("Bad format of config file!");
        }
        std::filesystem::path pluginsDir = std::filesystem::path(line.erase(0, 19)).make_preferred();
        if (!std::filesystem::is_directory(pluginsDir)) {
            throw std::runtime_error("Path to plugins dir is invalid!");
        }
        std::filesystem::path pluginRootDir = pluginsDir / std::filesystem::path(path).make_preferred();
        // Look case-insensitive for any "resources" dir.
        for (auto& name : std::filesystem::directory_iterator(pluginRootDir)) {
            if (!std::filesystem::is_directory(name.path())) {
                continue;
            }
            std::string nameLower = name.path().filename().string();
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                [](unsigned char c) { return std::tolower(c); });
            if (nameLower == "resources") {
                return name.path();
            }
        }
        throw std::runtime_error("Resources dir not found!");
    } else {
        // running from installed directory
        std::filesystem::path pluginResourcesDir =
            fullExeName.parent_path().parent_path() / "resources" / std::filesystem::path(path).make_preferred();
        if (!std::filesystem::is_directory(pluginResourcesDir)) {
            throw std::runtime_error("Resources dir not found!");
        }
        return pluginResourcesDir;
    }
}
