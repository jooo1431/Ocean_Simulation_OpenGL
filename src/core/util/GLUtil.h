#pragma once

#include <iostream>
#include <string>

#include <glad/gl.h>

#ifdef OGL4CORE2_ENABLE_STACKTRACE
#include <boost/stacktrace.hpp>
#endif

namespace OGL4Core2::Core {
    class GLUtil {
    public:
        static void printOpenGLInfo() {
            std::string glVersion(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
            std::string glVendor(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
            std::string glRenderer(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
            std::string glslVersion(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

            std::cout << "GPU details:" << std::endl;
            std::cout << "    Vendor:         " << glVendor << std::endl;
            std::cout << "    GPU:            " << glRenderer << std::endl;
            std::cout << "    OpenGL version: " << glVersion << std::endl;
            std::cout << "    GLSL version:   " << glslVersion << std::endl;
        }

        static void GLAPIENTRY OpenGLMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
            [[maybe_unused]] GLsizei length, const GLchar* message, [[maybe_unused]] const void* userParam) {
            // clang-format off
            std::cerr << (type == GL_DEBUG_TYPE_ERROR ? "[OpenGL Error]" : "[OpenGL Debug]")
                      << "  Source: " << getSource(source)
                      << "  Type: " << getType(type)
                      << "  Severity: " << getSeverity(severity)
                      << "  Id: " << id
                      << "  Message: " << message
                      << std::endl;
            // clang-format on

#ifdef OGL4CORE2_ENABLE_STACKTRACE
            std::cerr << "Stacktrace:" << std::endl << boost::stacktrace::stacktrace() << std::endl;
#endif
        }

    private:
        static std::string getSource(GLenum source) {
            switch (source) {
                case GL_DEBUG_SOURCE_API:
                    return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    return "WINDOW_SYSTEM";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    return "SHADER_COMPILER";
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    return "THIRD_PARTY";
                case GL_DEBUG_SOURCE_APPLICATION:
                    return "APPLICATION";
                case GL_DEBUG_SOURCE_OTHER:
                    return "OTHER";
                default:
                    return "UNKNOWN";
            }
        }

        static std::string getType(GLenum type) {
            switch (type) {
                case GL_DEBUG_TYPE_ERROR:
                    return "ERROR";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    return "DEPRECATED_BEHAVIOR";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    return "UNDEFINED_BEHAVIOR";
                case GL_DEBUG_TYPE_PORTABILITY:
                    return "PORTABILITY";
                case GL_DEBUG_TYPE_PERFORMANCE:
                    return "PERFORMANCE";
                case GL_DEBUG_TYPE_MARKER:
                    return "MARKER";
                case GL_DEBUG_TYPE_PUSH_GROUP:
                    return "PUSH_GROUP";
                case GL_DEBUG_TYPE_POP_GROUP:
                    return "POP_GROUP";
                case GL_DEBUG_TYPE_OTHER:
                    return "OTHER";
                default:
                    return "UNKNOWN";
            }
        }

        static std::string getSeverity(GLenum severity) {
            switch (severity) {
                case GL_DEBUG_SEVERITY_HIGH:
                    return "HIGH";
                case GL_DEBUG_SEVERITY_MEDIUM:
                    return "MEDIUM";
                case GL_DEBUG_SEVERITY_LOW:
                    return "LOW";
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    return "NOTIFICATION";
                default:
                    return "UNKNOWN";
            }
        }
    };
} // namespace OGL4Core2::Core
