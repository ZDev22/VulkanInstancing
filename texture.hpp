#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace vulkan {
    class Device;
    class Pipeline;

    class Texture {
    public:
        Texture(Device& device, const std::string& filepath, VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorPool descriptorPool, Pipeline& pipeline);
        Texture(Device& device, const std::vector<std::string>& filepaths, VkDescriptorSetLayout descriptorSetLayout,
            VkDescriptorPool descriptorPool, Pipeline& pipeline); // New: Texture array
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        VkDescriptorSet getDescriptorSet() { return descriptorSet; }
        VkImageView getImageView() { return imageView; }
        VkSampler getSampler() { return sampler; }
        VkImageLayout getImageLayout() { return imageLayout; }

    private:
        void createDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void createTextureArray(const std::vector<std::string>& filepaths); // New: Texture array creation

        Device& device;
        Pipeline& pipeline;
        VkImageLayout imageLayout;
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;
        VkDescriptorSet descriptorSet;
        VkFormat imageFormat;
        bool isArray{ false }; // Flag for texture array
        uint32_t arrayLayers{ 1 }; // Number of layers
    };
}
