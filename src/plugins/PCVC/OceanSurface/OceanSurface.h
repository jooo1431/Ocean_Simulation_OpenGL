#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ios>
#include <algorithm>
#include <complex>
#include <cmath>
#include <math.h>

#include <glad/gl.h>
#include <glm/matrix.hpp>

#include "core/PluginRegister.h"
#include "core/RenderPlugin.h"
#include "core/camera/OrbitCamera.h"

#include <glm/gtx/string_cast.hpp>

namespace OGL4Core2::Plugins::PCVC::OceanSurface {

    class OceanSurface : public Core::RenderPlugin {
        REGISTERPLUGIN(OceanSurface, 96) // NOLINT

    public:
        static std::string name() {
            return "PCVC/OceanSurface";
        }

        explicit OceanSurface(const Core::Core& c);
        ~OceanSurface() override;

        void render() override;
        void resize(int width, int height) override;
        //void keyboard(Core::Key key, Core::KeyAction action, Core::Mods mods) override;
        //void mouseButton(Core::MouseButton button, Core::MouseButtonAction action, Core::Mods mods) override;
        //void mouseMove(double xpos, double ypos) override;
        void GaussianRandomVariable();
        void renderGUI();
        void initTexture();
        void initVA();
        void initShaders();
        void initSkybox();
        void initFFTData();
        void initGrid();

        // render functions
        void renderInitialSpectrum();
        void renderWaveAmplitude();
        void renderIFFT(GLuint texInp, GLuint texOut);
        void renderSkybox();
        void renderButterfly();
        void renderNormalMap();
        void renderPerlinNoise();

        GLuint createTexture(GLenum format, GLenum internalformat, const void* data);
        int32_t bitReverse(int32_t num, int32_t size);

        std::vector<float> randomGradient(int ix, int iy);
        float dotGridGradient(int ix, int iy, float x, float y);
        float Perlin(float x, float y);
        float interpolate(float a0, float a1, float w);
        void initPerlin();
        
    private:
        
        float windowWidth;
        float windowHeight;

        // GUI parameters
        glm::vec3 backgroundColor;
        int currGUItex;
        std::vector<GLuint> textures_GUI;
        const char* tex_list;
        std::shared_ptr<Core::OrbitCamera> camera; //!< view matrix
        glm::mat4 projMx;

        std::unique_ptr<glowl::Mesh> vaQuad;
        std::unique_ptr<glowl::Mesh> vaSkybox;
        std::unique_ptr<glowl::Mesh> vaOceanSurface;

        // shader program 
        std::unique_ptr<glowl::GLSLProgram> shaderQuad; // shaders for quad
        std::unique_ptr<glowl::GLSLProgram> shaderPSpectrum; // #1 compute shader for Phillips SPectrum
        std::unique_ptr<glowl::GLSLProgram> shaderAmplitude; // #2 compute shader for Wave Amplitude
        std::unique_ptr<glowl::GLSLProgram> shaderButterfly; // #3 compute shader for Twiddle factors and indices for butterfly opeation
        std::unique_ptr<glowl::GLSLProgram> shaderInverseFFT; // #4 compute shader for Butterfly operation of FFT
        std::unique_ptr<glowl::GLSLProgram> shaderPerlinNoise; // #5 compute shader for Inverse FFT
        std::unique_ptr<glowl::GLSLProgram> shaderOceanSurface; // shaders for ocean surface
        std::unique_ptr<glowl::GLSLProgram> shaderSkybox; // shaders for skybox
        std::unique_ptr<glowl::GLSLProgram> shaderNormalMap;       // shaders for skybox

        // texture
        GLuint texH0k;
        GLuint texH0minusk;
        GLuint texGaussRnd;
        GLuint texHkt_dx;
        GLuint texHkt_dy;
        GLuint texHkt_dz;
        GLuint texSlope_x;
        GLuint texSlope_z;
        GLuint texButterfly;
        GLuint texPingPong;
        GLuint texDispX;
        GLuint texDispY;
        GLuint texDispZ;
        GLuint texNormalX;
        GLuint texNormalZ;
        GLuint texNormalMap;
        GLuint texSkybox;
        GLuint texPerlin;

        // Ocean Surface variables
        float phillipsConst;
        glm::vec2 windDir;
        float windSpeed;
        float time;
        bool change;
        bool initial;
        int butterflyStages;
        float choppiness;
        float suppression;
        bool showWireframe;
        float waveHeight;
        float lightLong; 
        float lightLat;  
        
    };
} // namespace OGL4Core2::Plugins::PCVC::OceanSurface
