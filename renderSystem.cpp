#include "renderSystem.hpp"
#include <stdexcept>
#include <iostream>
#include <array>
#include <glm/gtc/matrix_transform.hpp>
#include "global.hpp"

namespace vulkan {
    RenderSystem::RenderSystem(Device& device, Window& window, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
        : device{ device }, descriptorSetLayout{ descriptorSetLayout }, window{ window } {
        createPipelineLayout();
        createPipeline(renderPass);
        std::cout << "RenderSystem created" << std::endl;
    }

    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void RenderSystem::initialize() {
        initializeSpriteData();
        createTextureArrayDescriptorSet();
    }

    void RenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(Push);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void RenderSystem::createPipeline(VkRenderPass renderPass) {
        pipeline = std::make_unique<Pipeline>(
            device,
            "triangle.vert.spv",
            "triangle.frag.spv",
            renderPass
        );
    }

    void RenderSystem::initializeSpriteData() {
        std::cout << "Calling initializeSpriteData...\n";

        if (sprites.size() == 0) {
            std::cerr << "Error: No sprites to initialize. Skipping buffer creation.\n";
            return;
        }
        else {
            std::cout << "Sprite count: " << sprites.size() << "\n";
        }

        std::vector<SpriteData> spriteData(sprites.size());
        for (size_t i = 0; i < sprites.size(); i++) {
            auto& sprite = sprites[i];
            spriteData[i].translation = sprite.transform.translation;
            float scale = sprite.transform.scale.x;
            spriteData[i].transform = glm::mat2(scale, 0.0f, 0.0f, scale);
            spriteData[i].color = sprite.color;
            spriteData[i].textureId = 0;
            spriteData[i].speed = sprite.transform.speed;
            spriteData[i].padding = 0.0f;
        }

        VkDeviceSize bufferSize = sizeof(SpriteData) * sprites.size();
        std::cout << "SSBO size: " << bufferSize << " bytes, SpriteData size: " << sizeof(SpriteData) << " bytes\n";

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        void* data;
        vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, spriteData.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device.device(), stagingBufferMemory);

        spriteDataBuffer = std::make_unique<Buffer>(
            device,
            bufferSize,
            1,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            device.properties.limits.minStorageBufferOffsetAlignment
        );

        device.copyBuffer(stagingBuffer, spriteDataBuffer->getBuffer(), bufferSize);

        vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
        vkFreeMemory(device.device(), stagingBufferMemory, nullptr);

        std::cout << "Initialized " << sprites.size() << " sprites in SSBO" << std::endl;
    }

    void RenderSystem::createTextureArrayDescriptorSet() {
        if (!spriteDataBuffer) {
            throw std::runtime_error("spriteDataBuffer is not initialized!");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pipeline->getDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        if (vkAllocateDescriptorSets(device.device(), &allocInfo, &spriteDataDescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor set!");
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = spriteDataBuffer->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(SpriteData) * vulkan::sprites.size();

        VkWriteDescriptorSet bufferWrite{};
        bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bufferWrite.dstSet = spriteDataDescriptorSet;
        bufferWrite.dstBinding = 0;
        bufferWrite.dstArrayElement = 0;
        bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bufferWrite.descriptorCount = 1;
        bufferWrite.pBufferInfo = &bufferInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vulkan::sprites[0].texture->getImageView();
        imageInfo.sampler = vulkan::sprites[0].texture->getSampler();

        VkWriteDescriptorSet imageWrite{};
        imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageWrite.dstSet = spriteDataDescriptorSet;
        imageWrite.dstBinding = 1;
        imageWrite.dstArrayElement = 0;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageWrite.descriptorCount = 1;
        imageWrite.pImageInfo = &imageInfo;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = { bufferWrite, imageWrite };
        vkUpdateDescriptorSets(device.device(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

        std::cout << "Descriptor set bound for " << vulkan::sprites.size() << " sprites" << std::endl;
    }

    void RenderSystem::renderSprites(VkCommandBuffer commandBuffer) {
        pipeline->bind(commandBuffer);
        if (sprites.empty()) {
            std::cerr << "No sprites to render" << std::endl;
            return;
        }
        auto model = sprites[0].model;
        if (!model) {
            std::cerr << "Invalid model for sprite rendering" << std::endl;
            return;
        }

        model->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipelineLayout(), 0, 1, &spriteDataDescriptorSet, 0, nullptr);

        Push push{};
        VkExtent2D extent = window.getExtent();
        float aspectRatio = static_cast<float>(extent.width) / extent.height;
        push.projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        vkCmdPushConstants(commandBuffer, pipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Push), &push);

        uint32_t instanceCount = static_cast<uint32_t>(std::min(sprites.size(), size_t(std::numeric_limits<uint32_t>::max())));
        model->draw(commandBuffer, instanceCount);
    }

    void RenderSystem::updateSprites(float deltaTime) {
        std::vector<SpriteData> spriteData(sprites.size());

        for (size_t i = 0; i < sprites.size(); i++) {
            auto& sprite = sprites[i];

            sprite.transform.translation += sprite.transform.speed * deltaTime;

            spriteData[i].translation = sprite.transform.translation;
            float scale = sprite.transform.scale.x;
            spriteData[i].transform = glm::mat2(scale, 0.0f, 0.0f, scale);
            spriteData[i].color = sprite.color;
            spriteData[i].textureId = 0;
            spriteData[i].speed = sprite.transform.speed;
            spriteData[i].padding = 0.0f;
        }

        VkDeviceSize bufferSize = sizeof(SpriteData) * sprites.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        device.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        void* data;
        vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, spriteData.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device.device(), stagingBufferMemory);

        device.copyBuffer(stagingBuffer, spriteDataBuffer->getBuffer(), bufferSize);

        vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
        vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
    }
}