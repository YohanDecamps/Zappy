/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** LightingPass
*/

#include "RenderPasses/LightingPass.hpp"
#include "Renderer/ShaderProgram.hpp"

#include "stb_image.h"
#include "glm/gtc/matrix_transform.hpp"

#include <filesystem>

LightingPass::LightingPass(std::shared_ptr<Window>& window) : m_window(window) {
    m_lightingProgram = std::make_unique<ShaderProgram>("../GUI/shaders/Lighting.vert", "../GUI/shaders/Lighting.frag");
    m_equiRectangularToCubemapProgram = std::make_unique<ShaderProgram>("../GUI/shaders/Cubemap.vert", "../GUI/shaders/EquiRectangularToCubemap.frag");
    m_irradianceProgram = std::make_unique<ShaderProgram>("../GUI/shaders/Cubemap.vert", "../GUI/shaders/Irradiance.frag");
    m_backgroundProgram = std::make_unique<ShaderProgram>("../GUI/shaders/Background.vert", "../GUI/shaders/Background.frag");


    // Capture framebuffer setup (used to capture the HDR environment map)
    glGenFramebuffers(1, &m_captureFBO);
    glGenRenderbuffers(1, &m_captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);


    // load the HDR environment map
    stbi_set_flip_vertically_on_load(static_cast<int>(true));
    int width = 0;
    int height = 0;
    int nrComponents = 0;
    float *data = stbi_loadf(std::filesystem::path("../assets/skybox.hdr").c_str(), &width, &height, &nrComponents, 0);
    if (data == nullptr)
        throw std::runtime_error("Failed to load HDR image.");

    glGenTextures(1, &m_hdrTexture);
    glBindTexture(GL_TEXTURE_2D, m_hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);


    // Setup the cubemap to render to a framebuffer
    glGenTextures(1, &m_envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    // Setup projection & view matrices to capture the cubemap faces
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0), 1.0, 0.1, 10.0);
    std::array<glm::mat4, 6> captureViews = {
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, 0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)),
        glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0))
    };


    // Render the equirectangular map to the cubemaps
    m_equiRectangularToCubemapProgram->use();
    m_equiRectangularToCubemapProgram->setInt("equirectangularMap", 0);
    m_equiRectangularToCubemapProgram->setMat4("proj", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        m_equiRectangularToCubemapProgram->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Utils::renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Create the irradiance map
    glGenTextures(1, &m_irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);


    // Render the irradiance map with a convolution shader
    m_irradianceProgram->use();
    m_irradianceProgram->setInt("environmentMap", 0);
    m_irradianceProgram->setMat4("proj", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        m_irradianceProgram->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Utils::renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth = 0;
    int scrHeight = 0;
    glfwGetFramebufferSize(window->getHandle(), &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);
}

LightingPass::~LightingPass() {
}

void LightingPass::bind(uint32_t positionTexture, uint32_t albedoTexture, uint32_t normalTexture, uint32_t pbrTexture, uint32_t ssaoTexture, uint32_t ssrTexture, const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& proj, int debugView) const noexcept {
    m_lightingProgram->use();

    m_lightingProgram->setInt("debugView", debugView);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    m_lightingProgram->setInt("positionMap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, albedoTexture);
    m_lightingProgram->setInt("albedoMap", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    m_lightingProgram->setInt("normalMap", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, pbrTexture);
    m_lightingProgram->setInt("pbrMap", 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    m_lightingProgram->setInt("ssaoMap", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, ssrTexture);
    m_lightingProgram->setInt("ssrMap", 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_irradianceMap);
    m_lightingProgram->setInt("irradianceMap", 6);

    m_lightingProgram->setVec3("camPos", camPos);

    glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Utils::renderQuad();


    // Render skybox
    m_backgroundProgram->use();
    m_backgroundProgram->setMat4("view", view);
    m_backgroundProgram->setMat4("proj", proj);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
    m_backgroundProgram->setInt("envMap", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    m_backgroundProgram->setInt("normalMap", 1);

    Utils::renderCube();
}