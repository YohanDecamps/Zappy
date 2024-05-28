/*
** EPITECH PROJECT, 2024
** zappy
** File description:
** LightingPass
*/

#pragma once

#include "ShaderProgram.hpp"
#include "Window.hpp"

#include <cstdint>
#include <memory>

class LightingPass {
    public:
        LightingPass(std::shared_ptr<Window>& window);
        ~LightingPass();

        LightingPass(const LightingPass&) = delete;
        LightingPass& operator=(const LightingPass&) = delete;

        LightingPass(LightingPass&&) = default;
        LightingPass& operator=(LightingPass&&) = default;

        void bind(uint32_t positionTexture, uint32_t albedoTexture, uint32_t normalTexture, uint32_t pbrTexture, uint32_t ssaoTexture) const noexcept;

    private:
        std::shared_ptr<Window> m_window;
        std::unique_ptr<ShaderProgram> m_shaderProgram;
};
