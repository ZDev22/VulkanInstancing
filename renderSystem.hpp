#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "model.hpp"
#include "texture.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "swapChain.hpp"

namespace vulkan {

    struct SpriteData {
        glm::vec2 translation;
        glm::mat2 transform;
        glm::vec3 color;
        uint32_t textureId; // Change to uint32_t
        glm::vec2 speed;
        float padding;
    };

    class RenderSystem {
    public:
        RenderSystem(Device& device, Window& window, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
        ~RenderSystem();
        RenderSystem(const RenderSystem&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;

        void initialize();
        void renderSprites(VkCommandBuffer commandBuffer);
        void updateSprites(float deltaTime);


    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
        void initializeSpriteData();
        void createTextureArrayDescriptorSet();

        Device& device;
        Window& window;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<Pipeline> pipeline;
        std::unique_ptr<Buffer> spriteDataBuffer;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet spriteDataDescriptorSet;
        VkDescriptorSet textureArrayDescriptorSet;
        std::vector<std::string> texturePaths;
    };
}