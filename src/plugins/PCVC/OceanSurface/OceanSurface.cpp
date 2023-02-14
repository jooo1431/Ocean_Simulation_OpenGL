#include "OceanSurface.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <random>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_stdlib.h>

#include "core/Core.h"
#include "core/util/ImGuiUtil.h"

#define M_PI 3.14159265358979323846
#define LOCAL_WORK_GROUP_SIZE 32
#define FFT_RESOLUTION 256
#define GRID_SIZE 256

using namespace OGL4Core2;
using namespace OGL4Core2::Plugins::PCVC::OceanSurface;

/**
 * @brief OceanSurface constructor
 */
OceanSurface::OceanSurface(const Core::Core& c)
    : Core::RenderPlugin(c),
      currGUItex(0),
      windowWidth(0.0f),
      windowHeight(0.0f),
      projMx(glm::mat4(1.0f)),
      windDir(glm::vec2(1.0f, 1.0f)),
      windSpeed(80.0f),
      phillipsConst(4.0f),
      change(true),
      initial(true),
      butterflyStages(int(log(FFT_RESOLUTION) / log(2))),
      choppiness(5.0f),
      waveHeight(1.0f),
      suppression(0.1f),
      showWireframe(false),
      lightLong(50.0f),
      lightLat(50.00f),
      backgroundColor(glm::vec3(0.2f, 0.2f, 0.2f)) {
      // Init Camera
      camera = std::make_shared<Core::OrbitCamera>(100.0f);
      core_.registerCamera(camera);
      // Enable depth testing.
      glEnable(GL_DEPTH_TEST);
    //initPerlin();
      initShaders();
      initFFTData();
      initTexture();
      initGrid();
      initSkybox();
      
      // GUI settings
      textures_GUI = {texH0k, texH0minusk, texHkt_dy, texHkt_dx, texHkt_dz, texDispY, texDispX, texDispZ,
          texNormalMap, texButterfly};
      tex_list = "H0k\0H0minusk\0Hkt_dy\0Hkt_dx\0Hkt_dz\0DisplacementY\0DisplacementX\0DisplacementZ\0NormalMap\0Butterfly\0";
}

/**
 * @brief OceanSurface destructor
 */
OceanSurface::~OceanSurface() {

    // Reset OpenGL state.
    glDisable(GL_DEPTH_TEST);
}

/**
 * @brief GUI rendering.
 */
void OceanSurface::renderGUI() {
    if (ImGui::CollapsingHeader("OceanSurface", ImGuiTreeNodeFlags_DefaultOpen)) {
        camera->drawGUI();
        if(ImGui::SliderFloat("Wind Speed", &windSpeed, 5.0f, 100.0f)) change = true;
        if(ImGui::SliderFloat("Suppression factor", &suppression, 0.001f, 10.0f)) change = true;
        ImGui::SliderFloat("Choppiness", &choppiness, 1.0f, 20.0f);
        ImGui::SliderFloat("Wave Height", &waveHeight, 0.5f, 20.0f);
        ImGui::Checkbox("Wireframe", &showWireframe);
        ImGui::SliderFloat("lightLong", &lightLong, 0.0f, 360.0f);
        ImGui::SliderFloat("lightLat", &lightLat, -90.0f, 90.0f);
        ImGui::Image((void*) (intptr_t) texPerlin, ImVec2(512, 512));
        ImGui::Combo("Show Textures", &currGUItex, tex_list);
        if (textures_GUI[currGUItex] == texButterfly)
            ImGui::Image((void*) (intptr_t) textures_GUI[currGUItex], ImVec2(30 * 512, 512));
        else
            ImGui::Image((void*) (intptr_t) textures_GUI[currGUItex], ImVec2(512, 512));

       
       
    }
}


/*
 * @brief HelloCube render callback.
 */
void OceanSurface::render() {

    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // compute the initial time-independent spectrum when change occurs
    if (change)
        renderInitialSpectrum();
    
    // compute butterfly factors for FFT operation, runs only once to create data
    if (initial) {
        renderButterfly();
        renderPerlinNoise();
    }
    // time-dependent wave amplitude
    renderWaveAmplitude();

    // IFFT computation
    renderIFFT(texHkt_dy, texDispY); // Height Field texture
    renderIFFT(texHkt_dx, texDispX);
    renderIFFT(texHkt_dz, texDispZ);
    renderIFFT(texSlope_x, texNormalX); // Normal Map texture
    renderIFFT(texSlope_z, texNormalZ);

    // normal map computation
    renderNormalMap();
   
    renderGUI();
    
    // render ocean surface
    glPolygonMode(GL_FRONT_AND_BACK, showWireframe ? GL_LINE : GL_FILL);
    projMx = glm::perspective(glm::radians(45.0f), (float) (windowWidth / windowHeight), 0.1f, 10000.0f);

    shaderOceanSurface->use();

    // texture setting
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texDispY);
    shaderOceanSurface->setUniform("dispY", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texDispX);
    shaderOceanSurface->setUniform("dispX", 2);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texDispZ);
    shaderOceanSurface->setUniform("dispZ", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
    shaderOceanSurface->setUniform("skybox", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texNormalMap);
    shaderOceanSurface->setUniform("normalMap", 5);

    shaderOceanSurface->setUniform("projMx", projMx);
    shaderOceanSurface->setUniform("modelMx", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,5.0f)));
    shaderOceanSurface->setUniform("viewMx", camera->viewMx());
    shaderOceanSurface->setUniform("choppiness", choppiness);
    shaderOceanSurface->setUniform("waveHeight", waveHeight);
    shaderOceanSurface->setUniform("camPos", glm::vec3(glm::inverse(camera->viewMx())[3]));
    shaderOceanSurface->setUniform("invViewMx", glm::inverse(camera->viewMx()));
    glm::vec3 lightDir = glm::vec3(cosf(glm::radians(lightLat)) * cosf(glm::radians(lightLong)),
        cosf(glm::radians(lightLat)) * sinf(glm::radians(lightLong)), sinf(glm::radians(lightLat)));
    shaderOceanSurface->setUniform("lightDir", lightDir);
    vaOceanSurface->draw();

    // render cubemap texture
    renderSkybox();
}

void OceanSurface::renderPerlinNoise() {

    shaderPerlinNoise->use();
    glBindImageTexture(0, texPerlin, 0, GL_FALSE, 0, GL_READ_WRITE,GL_RGBA32F);
    shaderPerlinNoise->setUniform("N", FFT_RESOLUTION);
    float frequency = 1.0f;
    float persistence = 0.15; // 0~1
    float amplitude = 1.0f;
   
    for (int i = 0; i < 8; i++) { // loop through octaves
        shaderPerlinNoise->setUniform("frequency", frequency);
        shaderPerlinNoise->setUniform("amplitude", amplitude);

        glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        frequency *= 2.0f;
        amplitude *= persistence;
    }

}

/*
 * @brief Render skybox for background
 */
void OceanSurface::renderSkybox() { // render it as last in render()
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // since the cubemap will always have a depth of 1.0, we need the equal sign so it doesn#t get discarded
    glDepthFunc(GL_LEQUAL);
    shaderSkybox->use();

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
    shaderSkybox->setUniform("skybox", 6);

    shaderSkybox->setUniform("projMx", projMx);
    shaderSkybox->setUniform("modelMx", glm::mat4(1.0f));
    // remove the translation of view matrix to keep skybox surrounded by the camera, i.e, worldpos is set at camera,
    // always keep camera at the center
    shaderSkybox->setUniform("viewMx", glm::mat4(glm::mat3(camera->viewMx())));

    vaSkybox->draw();
    glDepthFunc(GL_LESS); // set depth back to default
    glUseProgram(0);
}

/*
 * @brief Compute Normalmap
 */
void OceanSurface::renderNormalMap() {

    shaderNormalMap->use();
    glBindImageTexture(0, texDispY, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texNormalX, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(2, texNormalZ, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(3, texNormalMap, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    shaderNormalMap->setUniform("N", FFT_RESOLUTION);
    shaderNormalMap->setUniform("choppiness", choppiness);
    shaderNormalMap->setUniform("waveHeight", waveHeight);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, texDispY);
    shaderNormalMap->setUniform("height", 7);

    glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);
}

 /*
 * @brief Compute displacement field by IFFT computation
 */
void OceanSurface::renderIFFT(GLuint texInp, GLuint texOut) {

    shaderInverseFFT->use();
    glBindImageTexture(0, texButterfly, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F); // read precomputed data for butterfly operation
    glBindImageTexture(1, texInp, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // initial texture to read from
    glBindImageTexture(2, texPingPong, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F); // pingpong texture to write to
    glBindImageTexture(3, texOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // final output texture
    shaderInverseFFT->setUniform("inv", false);
    int pingPong = 0;

    // 1D FFT Horizontal
    for (int i = 0; i < butterflyStages; i++) {

        shaderInverseFFT->setUniform("direction", 0);
        shaderInverseFFT->setUniform("stage", i);
        shaderInverseFFT->setUniform("pingpong", pingPong);

        // run the compute shader each butterfly step
        glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        pingPong++;
        pingPong = pingPong % 2;
    }

    // output texture of the horizontal 1D FFT is the input for the vertical phase
    // 1D FFT Vertical
    for (int i = 0; i < butterflyStages; i++) {

        shaderInverseFFT->setUniform("direction", 1);
        shaderInverseFFT->setUniform("stage", i);
        shaderInverseFFT->setUniform("pingpong", pingPong);

        // run the compute shader each step
        glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        pingPong++;
        pingPong = pingPong % 2;
    }

    // inverse the FFT
    shaderInverseFFT->setUniform("inv", true);
    shaderInverseFFT->setUniform("pingpong", pingPong);
    shaderInverseFFT->setUniform("N", FFT_RESOLUTION);
    glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

/*
 * @brief Compute data for Butterfly operation for FFT computation
 */
void OceanSurface::renderButterfly() {

    shaderButterfly->use();
    shaderButterfly->setUniform("N", FFT_RESOLUTION);
    glBindImageTexture(0, texButterfly, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(butterflyStages, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);
    initial = false;
}

/*
 * @brief Compute time-dependent Wave amplitude h(k,t)
 */
void OceanSurface::renderWaveAmplitude() {

    shaderAmplitude->use();
    shaderAmplitude->setUniform("N", FFT_RESOLUTION);
    shaderAmplitude->setUniform("len", 1000.0f);
    shaderAmplitude->setUniform("t", float(glfwGetTime()));
    //std::cout << glfwGetTime() << std::endl;
    
    // displacement
    glBindImageTexture(0, texHkt_dy, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texHkt_dx, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, texHkt_dz, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // normal
    glBindImageTexture(3, texSlope_x, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(4, texSlope_z, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    // initial data
    glBindImageTexture(5, texH0k, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(6, texH0minusk, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);
}

/*
 * @brief Compute time-indpendent h(k) for the initial spectrum computation
 */
void OceanSurface::renderInitialSpectrum() {

    shaderPSpectrum->use();

    // Gaussian Random Variable
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texGaussRnd);
    shaderPSpectrum->setUniform("gaussRnd", 0);

    shaderPSpectrum->setUniform("N", FFT_RESOLUTION);
    shaderPSpectrum->setUniform("len", 1000.0f);
    shaderPSpectrum->setUniform("A", phillipsConst);
    shaderPSpectrum->setUniform("windDir", windDir);
    shaderPSpectrum->setUniform("windSpeed", windSpeed);
    shaderPSpectrum->setUniform("l", suppression);

    glBindImageTexture(0, texH0k, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, texH0minusk, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // processing 512/32 x 512/32 work groups in parallell in the GPU
    glDispatchCompute(FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, FFT_RESOLUTION / LOCAL_WORK_GROUP_SIZE, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // flag to stop rendering after running once
    change = false;
    glUseProgram(0);
}

/**
 * @brief Window resize callback.
 * @param width
 * @param height
 */
void OceanSurface::resize(int width, int height) {

    //update window size in the projection matrix
    windowWidth = (float) width;
    windowHeight = (float) height;

    //change viewport according to the new window size
    glViewport(0, 0, width, height);
}

/*
 * @brief Generate Gaussian distributed random number as a texture data for GPU
 */
void OceanSurface::GaussianRandomVariable() {

    std::random_device rd{};
    
    //std::mt19937 generator;
    std::minstd_rand generator{rd()};
  
    // gaussian random variable with mean 0.0 and standard deviation 1.0
    std::normal_distribution<float> nd(0.0, 1.0);

    std::vector<float> rndData;

    for (int i = 0; i < FFT_RESOLUTION; i++) {
        for (int j = 0; j < FFT_RESOLUTION; j++) {
            rndData.push_back(nd(generator));
            rndData.push_back(nd(generator));
            rndData.push_back(nd(generator));
            rndData.push_back(nd(generator));
        }
    }

    // create texture to load data and use it in the GPU
    texGaussRnd = createTexture(GL_RGBA, GL_RGBA32F, rndData.data());
}

/*
 * @brief Reverse bits for the butterfly operation
 */
int32_t OceanSurface::bitReverse(int32_t num, int32_t size) {

    int32_t reversed, i;

    for (reversed = 0, i = 0; i < size; ++i) {
        reversed |= ((num >> i) & 1) << (size - i - 1);
    }

    return reversed;
}

/*
 * @brief compute relavant data in the CPU to use it in the GPU 
 */
void OceanSurface::initFFTData() {

    GaussianRandomVariable();

    std::vector<int32_t> reversed;
    int32_t size = log(FFT_RESOLUTION) / log(2);

    for (int i = 0; i < FFT_RESOLUTION; i++) {
        reversed.push_back(bitReverse(i, size));
    }

    GLuint bitReversed;

    glGenBuffers(1, &bitReversed);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bitReversed);

    // upload data to ssb
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int32_t) * reversed.size(), reversed.data(), GL_STATIC_READ);

    // bind ssb by its base address to be accessible from a shader
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bitReversed);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    std::vector<float> maxMinHeight;
    maxMinHeight.push_back(0.0f);
    maxMinHeight.push_back(0.0f);

    GLuint data;
    glGenBuffers(1, &data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data);

    // upload data to ssb
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * 2, maxMinHeight.data(), GL_DYNAMIC_COPY);

    // bind ssb by its base address to be accessible from a shader
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

/*
 * @brief Init textures
 */
void OceanSurface::initTexture() {

    // initial spectrum data
    texH0k = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texH0minusk = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    // time-dependent spectrum data
    texHkt_dx = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texHkt_dy = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texHkt_dz = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texSlope_x = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texSlope_z = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    // FFT computation
    texButterfly = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texPingPong = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texDispY = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texDispX = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texDispZ = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texNormalX = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texNormalZ = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texNormalMap = createTexture(GL_RGBA, GL_RGBA32F, NULL);
    texPerlin = createTexture(GL_RGBA, GL_RGBA32F, NULL);
}

/*
 * @brief Create texure and return it
 */
GLuint OceanSurface::createTexture(GLenum format, GLenum internalformat, const void* data) {

    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR outputs less wavy waves
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // create empty texture
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, FFT_RESOLUTION, FFT_RESOLUTION, 0, format, GL_FLOAT, data);

    // release after use
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

/*
 * @brief Init skybox settings
 */
void OceanSurface::initSkybox() {
    
    const std::vector<float> skyboxVertices{
        -1.0f, -1.0f, -1.0f, // left bottom back
        1.0f, -1.0f, -1.0f,  // right bottom back
        -1.0f, 1.0f, -1.0f,  // left top back
        1.0f, 1.0f, -1.0f,   // right top back
        -1.0f, -1.0f, 1.0f,  // left bottom front
        1.0f, -1.0f, 1.0f,   // right bottom front
        -1.0f, 1.0f, 1.0f,   // left top front
        1.0f, 1.0f, 1.0f,    // right top front
    };

    const std::vector<GLuint> skyboxIndices{
        0, 2, 1, 1, 2, 3, // back
        5, 7, 4, 4, 7, 6, // front
        1, 5, 0, 0, 5, 4, // bottom
        7, 3, 6, 6, 3, 2, // top
        4, 6, 0, 0, 6, 2, // left
        1, 3, 5, 5, 3, 7  // right
    };

    glowl::Mesh::VertexDataList<float> vertexDataSkybox{{skyboxVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}}};
    vaSkybox = std::make_unique<glowl::Mesh>(vertexDataSkybox, skyboxIndices, GL_UNSIGNED_INT, GL_TRIANGLES);
   
    std::string skyboxes[6] = {"right.png", "left.png", "top.png", "bottom.png", "front.png", "back.png"};
    
    glGenTextures(1, &texSkybox);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texSkybox);
    // texture options
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // load data to cubemap for each directions
    for (int i = 0; i < 6; i++) {
        std::string path = "skybox1/" + skyboxes[i];
        int width, height;
        auto image = getPngResource(path, width, height);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
            (const void*) &image[0]);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

/*
 * @brief Create basic mesh for ocean surface
 */
void OceanSurface::initGrid() {

    std::vector<GLuint> oceanIndices;
    std::vector<float> oceanVertices;
    std::vector<float> oceanTexCoord;

    int vertSize = GRID_SIZE + 1;

    float x0 = -GRID_SIZE / 2.0f; 
    float z0 = -GRID_SIZE / 2.0f; 
    
    float texCoordScale = float(GRID_SIZE) / float(FFT_RESOLUTION);

    for (int j = 0; j <= GRID_SIZE; j++) {
        for (int i = 0; i <= GRID_SIZE; i++) {
           
            // vertices
            oceanVertices.push_back(float(x0 + i));
            //oceanVertices.push_back(float((i - GRID_SIZE / 2) * 1000 / GRID_SIZE));
            oceanVertices.push_back(0.0f); // initial height 0
            oceanVertices.push_back(float(z0 + j));
            //oceanVertices.push_back(float((j - GRID_SIZE / 2) * 1000 / GRID_SIZE));

            // tex coord
            oceanTexCoord.push_back(texCoordScale * float(i) / float(GRID_SIZE)); // u
            oceanTexCoord.push_back(texCoordScale * float(j) / float(GRID_SIZE)); // v
        }
    }

    for (int j = 0; j < GRID_SIZE; j++) {
        for (int i = 0; i < GRID_SIZE; i++) {

            int i0 = (j * vertSize) + i;
            int i1 = ((j + 1) * vertSize) + i;

            // upper-left triangle
            oceanIndices.push_back(i0);
            oceanIndices.push_back(i1 + 1);
            oceanIndices.push_back(i1);
            // bottom-right triangle
            oceanIndices.push_back(i0);
            oceanIndices.push_back(i0 + 1);
            oceanIndices.push_back(i1 + 1);
        }
    }
    std::cout << "oceanVertices: " << oceanVertices.size() / 3.0f<< std::endl;
    std::cout << "oceanTexCoord : " << oceanTexCoord.size() / 2.0f  << std::endl;
    std::cout << "oceanIndices: " << oceanIndices.size() << std::endl;

    glowl::Mesh::VertexDataList<float> vertexDataOcean{
        {oceanVertices, {12, {{3, GL_FLOAT, GL_FALSE, 0}}}},
        {oceanTexCoord, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}}};
    vaOceanSurface = std::make_unique<glowl::Mesh>(vertexDataOcean, oceanIndices, GL_UNSIGNED_INT, GL_TRIANGLES);
}

void OceanSurface::initVA() {
    //  Create a vertex array for the window filling quad.
    std::vector<float> quadVertices{
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    std::vector<uint32_t> quadIndices{
        0, 1, 2,
        2, 1, 3
    };
    glowl::Mesh::VertexDataList<float> vertexDataQuad{{quadVertices, {8, {{2, GL_FLOAT, GL_FALSE, 0}}}}};
    vaQuad = std::make_unique<glowl::Mesh>(vertexDataQuad, quadIndices);
}

/*
 * @brief Init shader programs
 */
void OceanSurface::initShaders() {

    try {
        shaderOceanSurface = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/OceanSurface.vert")},
           // {glowl::GLSLProgram::ShaderType::Geometry, getStringResource("shaders/OceanSurface.geom")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/OceanSurface.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderSkybox = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Vertex, getStringResource("shaders/Skybox.vert")},
            {glowl::GLSLProgram::ShaderType::Fragment, getStringResource("shaders/Skybox.frag")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    // shader program for compute shaders
    try {
        shaderPSpectrum = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Compute, getStringResource("shaders/PhillipsSpectrum.comp")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderAmplitude = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Compute, getStringResource("shaders/WaveAmplitude.comp")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderButterfly = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Compute, getStringResource("shaders/ButterflyFactor.comp")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderInverseFFT = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
             {glowl::GLSLProgram::ShaderType::Compute, getStringResource("shaders/InverseFFT.comp")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderPerlinNoise = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Compute, getStringResource("shaders/PerlinNoise.comp")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }

    try {
        shaderNormalMap = std::make_unique<glowl::GLSLProgram>(glowl::GLSLProgram::ShaderSourceList{
            {glowl::GLSLProgram::ShaderType::Compute, getStringResource("shaders/NormalMap.comp")}});
    } catch (glowl::GLSLProgramException& e) {
        std::cerr << e.what() << std::endl;
    }
}

