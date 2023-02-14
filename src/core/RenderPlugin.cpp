#include "RenderPlugin.h"

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <lodepng.h>

#include "Core.h"

using namespace OGL4Core2::Core;

RenderPlugin::RenderPlugin(const Core& c) : core_(c) {}

void RenderPlugin::resize([[maybe_unused]] int width, [[maybe_unused]] int height) {}

void RenderPlugin::keyboard([[maybe_unused]] Key key, [[maybe_unused]] KeyAction action, [[maybe_unused]] Mods mods) {}

void RenderPlugin::charInput([[maybe_unused]] unsigned int codepoint) {}

void RenderPlugin::mouseButton([[maybe_unused]] MouseButton button, [[maybe_unused]] MouseButtonAction action,
    [[maybe_unused]] Mods mods) {}

void RenderPlugin::mouseMove([[maybe_unused]] double xpos, [[maybe_unused]] double ypos) {}

void RenderPlugin::mouseScroll([[maybe_unused]] double xoffset, [[maybe_unused]] double yoffset) {}

std::filesystem::path RenderPlugin::getResourcePath(const std::string& name) const {
    auto basePath = core_.getPluginResourcesPath();

    // Replace '\' with '/' in case Windows style path separation is used instead of generic format '/'.
    std::string nameClean = name;
    std::replace(nameClean.begin(), nameClean.end(), '\\', '/');

    return basePath / std::filesystem::path(nameClean).make_preferred();
}

std::filesystem::path RenderPlugin::getResourceFilePath(const std::string& name) const {
    std::filesystem::path path = getResourcePath(name);
    if (!std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("Invalid resource file name: \"" + name + "\"! Path \"" + path.string() +
                                 "\" does not exists or is not a file.");
    }
    return path;
}

std::filesystem::path RenderPlugin::getResourceDirPath(const std::string& name) const {
    auto path = getResourcePath(name);
    if (!std::filesystem::is_directory(path)) {
        throw std::runtime_error("Invalid resource dir name: \"" + name + "\"! Path \"" + path.string() +
                                 "\" does not exist or is not a directory.");
    }
    return path;
}

std::string RenderPlugin::getStringResource(const std::string& name) const {
    std::filesystem::path path = getResourceFilePath(name);
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot read resource file \"" + path.string() + "\"!");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<unsigned char> RenderPlugin::getPngResource(const std::string& name, int& width, int& height) const {
    std::vector<unsigned char> image;
    unsigned int w, h;
    unsigned int error = lodepng::decode(image, w, h, getResourceFilePath(name).string());
    if (error != 0) {
        std::string errorText = lodepng_error_text(error);
        throw std::runtime_error("Cannot load PNG resource: " + errorText);
    }
    width = static_cast<int>(w);
    height = static_cast<int>(h);
    return image;
}

std::shared_ptr<glowl::Texture2D> RenderPlugin::getTextureResource(const std::string& name) const {
    int width, height;
    std::vector<unsigned char> image = getPngResource(name, width, height);
    glowl::TextureLayout layout(GL_RGBA8, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, 1,
        {
            {GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE},
            {GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE},
            {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
            {GL_TEXTURE_MAG_FILTER, GL_LINEAR},
        },
        {});
    return std::make_shared<glowl::Texture2D>(name, layout, image.data());
}

std::vector<std::filesystem::path> RenderPlugin::getResourceDirFilePaths(const std::string& name,
    const std::string& filter) const {
    std::filesystem::path dir = getResourceDirPath(name);
    std::vector<std::filesystem::path> files;
    std::regex filterRegex(filter);
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        std::filesystem::directory_entry a;
        if (!std::filesystem::is_regular_file(entry)) {
            continue;
        }
        if (!filter.empty() && !std::regex_match(entry.path().filename().string(), filterRegex)) {
            continue;
        }
        files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());
    return files;
}
