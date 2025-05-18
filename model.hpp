#pragma once
#include "device.hpp"
#include "buffer.hpp"
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory> // Added for std::unique_ptr

namespace vulkan {
    class Model {
    public:
        struct Vertex {
            glm::vec2 position;
            glm::vec3 color;
            glm::vec2 texCoord;

            static VkVertexInputBindingDescription getBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        Model(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~Model();

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer, uint32_t instanceCount);

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        Device& device;
        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;
        bool hasIndexBuffer;
    };
}