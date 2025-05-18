#pragma once
#include <glm/glm.hpp>

namespace vulkan {
    struct Push {
        glm::mat4 projection;
    };
    extern std::vector<Sprite> sprites;
}