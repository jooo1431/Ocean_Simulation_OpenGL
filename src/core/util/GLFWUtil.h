#pragma once

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace OGL4Core2::Core {
    class GLFWUtil {
    public:
        /**
         * GLFW mods parameter is not platform independent, see https://github.com/glfw/glfw/issues/1630. This function
         * should be called within the glfw keyboard callback, to set mods to uniform behavior across platforms. We
         * want to provide the modifier state "after" the key event here. That means the shift press event will have
         * set the shift modifier to true.
         *
         * @param mods
         * @param key
         * @param action
         * @return mods
         */
        static int fixKeyboardMods(int mods, int key, int action) {
            if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
                return (action == GLFW_RELEASE) ? mods & (~GLFW_MOD_SHIFT) : mods | GLFW_MOD_SHIFT;
            }
            if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
                return (action == GLFW_RELEASE) ? mods & (~GLFW_MOD_CONTROL) : mods | GLFW_MOD_CONTROL;
            }
            if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT) {
                return (action == GLFW_RELEASE) ? mods & (~GLFW_MOD_ALT) : mods | GLFW_MOD_ALT;
            }
            if (key == GLFW_KEY_LEFT_SUPER || key == GLFW_KEY_RIGHT_SUPER) {
                return (action == GLFW_RELEASE) ? mods & (~GLFW_MOD_SUPER) : mods | GLFW_MOD_SUPER;
            }
            return mods;
        }

        /**
         * Check if any modifier key (shift, control, alt, super) is currently pressed. This check utilises glfwGetKey()
         * and therefore only can return the cached key state, which misses when a key was pressed, while the window was
         * not in focus, but the key is still hold now.
         *
         * @param window
         * @return bool
         */
        static bool anyModKeyPressed(GLFWwindow* window) {
            return glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
        }
    };
} // namespace OGL4Core2::Core
