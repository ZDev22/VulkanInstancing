#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "model.hpp"
#include "texture.hpp"

namespace vulkan {
    struct Sprite {
        std::shared_ptr<Model> model;
        Texture* texture;
        glm::vec3 color;
        struct Transform2dComponent {
            glm::vec2 translation{ 0.f, 0.f };
            glm::vec2 scale{ 1.f, 1.f };
            float rotation{ 0.0f };
            glm::vec2 speed{ 0.0f, 0.0f };
        } transform;
    };
}