#pragma once

#include <exception>
#include <filesystem>
#include <memory>
#include <vector>

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "camera/AbstractCamera.h"
#include "Input.h"
#include "util/FpsCounter.h"

namespace OGL4Core2::Core {
    class RenderPlugin;

    class Core {
    public:
        Core();
        ~Core();

        void run();

        [[nodiscard]] std::filesystem::path getPluginResourcesPath() const;

        [[nodiscard]] bool isKeyPressed(Key key) const;
        [[nodiscard]] bool isMouseButtonPressed(MouseButton button) const;
        void getMousePos(double& xpos, double& ypos) const;

        void setWindowSize(int width, int height) const;

        void registerCamera(const std::shared_ptr<AbstractCamera>& camera) const;
        void removeCamera() const;

    private:
        void validateImGuiScale();
        void draw();

        void windowSizeEvent(int width, int height);
        void framebufferSizeEvent(int width, int height);
        void keyEvent(int key, int scancode, int action, int mods);
        void charEvent(unsigned int codepoint);
        void mouseButtonEvent(int button, int action, int mods);
        void mouseMoveEvent(double xpos, double ypos);
        void mouseScrollEvent(double xoffset, double yoffset);

        void scaleWindowPosToFramebufferPos(double& xpos, double& ypos) const;

        GLFWwindow* window_;
        bool running_;

        FpsCounter fps_;

        std::shared_ptr<RenderPlugin> currentPlugin_;
        std::filesystem::path currentPluginResourcesPath_;
        std::exception currentPluginResourcesPathException_;
        int currentPluginIdx_;
        int pluginSelectionIdx_;
        std::vector<char> pluginNamesImGui_;

        // Only for Win32 and X11 systems window size to framebuffer size is 1:1, according to GLFW doc. Within this
        // application we want to ignore high DPI scaling - at least for now - and handle only one size within the
        // plugins as main coordinate system. Because we are targeting OpenGL development, we use the framebuffer size
        // for this. Nevertheless, GLFW will return the mouse position in window coordinates. Therefore, we need to map
        // this input to the framebuffer coordinate system. This requires to also manage the window size changes.
        int windowWidth_;
        int windowHeight_;
        int framebufferWidth_;
        int framebufferHeight_;
        float contentScale_;
        double mouseX_;
        double mouseY_;

        AbstractCamera::MouseControlMode cameraControlMode_;
        mutable std::weak_ptr<AbstractCamera> camera_;

        static void initGLFW();
        static void terminateGLFW();

        static int glfwReferenceCounter_;
    };
} // namespace OGL4Core2::Core
