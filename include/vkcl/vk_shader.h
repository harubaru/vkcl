#ifndef VK_SHADER_H
#define VK_SHADER_H

#include "util_exception.h"
#include "vk_device.h"
#include "vk_buffer.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

namespace vkcl {

	class Shader {
	public:
		Shader() : freed(true) {}
		Shader(const Device &dev, const std::string fp, size_t BufferCount) { Load(dev, fp, BufferCount); }
		void Load(const Device &dev, const std::string fp, size_t BufferCount);
		void Delete();
		~Shader();

		void BindBuffers(Buffer *buffers);
		void Run(uint32_t x, uint32_t y, uint32_t z);
	protected:
		bool freed;
		size_t BufferCount;
		Device dev;
		VkCommandBuffer commandbuffer;
		VkShaderModule shadermod;
		VkPipeline pipeline;
		VkPipelineLayout pipelinelayout;
		VkDescriptorPool pool;
		VkDescriptorSet set;
		VkDescriptorSetLayout layout;
		VkFence fence;
	};

}

#endif
