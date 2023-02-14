#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <glad/gl.h>
#include <glowl/glowl.h>

#include "Input.h"

namespace OGL4Core2::Core {
    class Core;

    class RenderPlugin {
    public:
        explicit RenderPlugin(const Core& c);
        virtual ~RenderPlugin() = default;

        virtual void render() = 0;

        virtual void resize(int width, int height);
        virtual void keyboard(Key key, KeyAction action, Mods mods);
        virtual void charInput(unsigned int codepoint);
        virtual void mouseButton(MouseButton button, MouseButtonAction action, Mods mods);
        virtual void mouseMove(double xpos, double ypos);
        virtual void mouseScroll(double xoffset, double yoffset);

        [[nodiscard]] std::filesystem::path getResourcePath(const std::string& name) const;
        [[nodiscard]] std::filesystem::path getResourceFilePath(const std::string& name) const;
        [[nodiscard]] std::filesystem::path getResourceDirPath(const std::string& name) const;
        [[nodiscard]] std::string getStringResource(const std::string& name) const;
        [[nodiscard]] std::vector<unsigned char> getPngResource(const std::string& name, int& width, int& height) const;
        [[nodiscard]] std::shared_ptr<glowl::Texture2D> getTextureResource(const std::string& name) const;
        [[nodiscard]] std::vector<std::filesystem::path> getResourceDirFilePaths(const std::string& name,
            const std::string& filter = std::string()) const;

    protected:
        const Core& core_;
    };
} // namespace OGL4Core2::Core
