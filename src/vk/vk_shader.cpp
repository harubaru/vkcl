#include <vkcl/vk_shader.h>

namespace vkcl {
	
	// spirv bytecode reader helper
	static uint32_t *readspv(uint32_t *len, const std::string fp)
	{
		FILE *f = fopen(fp.c_str(), "rb");
		if ((!f) || (!len)) {
			throw vkcl::util::Exception(std::string("Could not open SPIR-V file: ") + fp);
			return nullptr;
		}

		// filesize
		fseek(f, 0, SEEK_END);
		uint32_t size = (unsigned)ftell(f);
		fseek(f, 0, SEEK_SET);
		uint32_t size_padded = (uint32_t)ceil(size / 4.0) * 4;
		*len = size_padded;

		// read file
		char *data = (char *)malloc(size_padded);
		if (data == NULL) {
			throw vkcl::util::Exception(std::string("Could not allocate data for SPIR-V file: ") + fp);
			return nullptr;
		}

		fread(data, size, 1, f);
		fclose(f);

		// pad data
		for (auto i = size; i < size_padded; i++) {
			data[i] = 0;
		}

		return (uint32_t *)data;
	}

	void Shader::Load(const Device &dev, const std::string fp, size_t BufferCount)
	{
		freed = false;
		this->dev.Load(dev);
		this->BufferCount = BufferCount;

		// initialize descriptor set layout
		VkDescriptorSetLayoutBinding *bindings = (VkDescriptorSetLayoutBinding *)malloc(sizeof(VkDescriptorSetLayoutBinding) * BufferCount);
		if (!bindings) {
			throw vkcl::util::Exception("Failed to allocate memory for Descriptor Set Layout Bindings");
		}

		for (size_t i = 0; i < BufferCount; i++) { // boiler plate code is for retards
			bindings[i] = {};
			bindings[i].binding = i; // 0 : input, 1 : output
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[i].pImmutableSamplers = nullptr;
		}

		// struct for holding layout bindings to create layout object
		VkDescriptorSetLayoutCreateInfo layoutcreateinfo = {};
		layoutcreateinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutcreateinfo.bindingCount = BufferCount;
		layoutcreateinfo.pBindings = bindings;

		if (vkCreateDescriptorSetLayout(this->dev.get(), &layoutcreateinfo, NULL, &layout) != VK_SUCCESS) {
			throw vkcl::util::Exception("Could not create descriptor set layout");
		}

		free(bindings);
		
		VkDescriptorPoolSize poolsize = {};
		poolsize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolsize.descriptorCount = BufferCount;

		VkDescriptorPoolCreateInfo poolcreateinfo = {};
		poolcreateinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolcreateinfo.maxSets = 1;
		poolcreateinfo.poolSizeCount = 1;
		poolcreateinfo.pPoolSizes = &poolsize;

		if (vkCreateDescriptorPool(this->dev.get(), &poolcreateinfo, NULL, &pool) != VK_SUCCESS) {
			throw vkcl::util::Exception("Could not create descriptor pool");
		}
		
		// allocate descriptor set
		VkDescriptorSetAllocateInfo allocinfo = {};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = pool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = &layout;

		if (vkAllocateDescriptorSets(this->dev.get(), &allocinfo, &set) != VK_SUCCESS) {
			throw vkcl::util::Exception("Could not allocate descriptor set ");
		}

		// Create shader module
		uint32_t len = 0;
		uint32_t *code = nullptr;
		
		try { 
			code = readspv(&len, fp);
		} catch (vkcl::util::Exception& e) {
			throw e;
		}

		if (!code)
			throw vkcl::util::Exception("Could not load shader code");

		VkShaderModuleCreateInfo modcreateinfo = {};
		modcreateinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		modcreateinfo.pCode = code;
		modcreateinfo.codeSize = len;

		if (vkCreateShaderModule(this->dev.get(), &modcreateinfo, NULL, &shadermod) != VK_SUCCESS) {
			throw vkcl::util::Exception("Could not allocate shader module");
		}

		free(code);

		// Create pipeline
		VkPipelineLayoutCreateInfo pipelinelayoutcreateinfo = {};
		pipelinelayoutcreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelinelayoutcreateinfo.pNext = nullptr;
		pipelinelayoutcreateinfo.setLayoutCount = 1;
		pipelinelayoutcreateinfo.pSetLayouts = &layout;
		
		if (vkCreatePipelineLayout(this->dev.get(), &pipelinelayoutcreateinfo, nullptr, &pipelinelayout) != VK_SUCCESS) {
			throw vkcl::util::Exception("Could not create Compute Pipeline Layout");
		}
		
		VkPipelineShaderStageCreateInfo shaderstagecreateinfo = {}; 
		shaderstagecreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderstagecreateinfo.pNext = nullptr;
		shaderstagecreateinfo.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT_EXT;
		shaderstagecreateinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderstagecreateinfo.module = shadermod;
		shaderstagecreateinfo.pName = "main";
		shaderstagecreateinfo.pSpecializationInfo = nullptr;

		VkComputePipelineCreateInfo pipelinecreateinfo = {};
		pipelinecreateinfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelinecreateinfo.pNext = nullptr;
		pipelinecreateinfo.stage = shaderstagecreateinfo;
		pipelinecreateinfo.layout = pipelinelayout;

		if (vkCreateComputePipelines(this->dev.get(), VK_NULL_HANDLE, 1, &pipelinecreateinfo, nullptr, &pipeline) != VK_SUCCESS) {
			throw vkcl::util::Exception("Could not create Compute Pipeline");
		}

		// Allocate command buffer from device
		VkCommandBufferAllocateInfo commandbufferinfo = {};
		commandbufferinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandbufferinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandbufferinfo.commandPool = this->dev.getCommandPool();
		commandbufferinfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(this->dev.get(), &commandbufferinfo, &commandbuffer) != VK_SUCCESS) {
			throw vkcl::util::Exception(std::string("Failed to allocate command buffer for compute operation")); 
		}

		VkFenceCreateInfo fenceinfo = {};
		fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceinfo.flags = 0;
		vkCreateFence(this->dev.get(), &fenceinfo, nullptr, &fence);
	}

	void Shader::Delete()
	{
		if (!freed) {
			vkDestroyFence(dev.get(), fence, nullptr);
			vkFreeCommandBuffers(dev.get(), dev.getCommandPool(), 1, &commandbuffer);
			vkDestroyPipeline(dev.get(), pipeline, nullptr);
			vkDestroyPipelineLayout(dev.get(), pipelinelayout, nullptr);
			vkDestroyShaderModule(dev.get(), shadermod, nullptr);
			vkDestroyDescriptorSetLayout(dev.get(), layout, nullptr);
			vkDestroyDescriptorPool(dev.get(), pool, nullptr);
			freed = true;
		}
	}

	Shader::~Shader()
	{
		Delete();
	}

	void Shader::BindBuffers(Buffer *buffers)
	{
		VkDescriptorBufferInfo *bufferinfo = (VkDescriptorBufferInfo *)malloc(sizeof(VkDescriptorBufferInfo) * BufferCount);

		for (size_t i = 0; i < BufferCount; i++) {
			bufferinfo[i].offset = 0;
			bufferinfo[i].range = VK_WHOLE_SIZE; // heh, what can possibly go wrong?
			bufferinfo[i].buffer = buffers[i].getDevBuffer();
		}

		VkWriteDescriptorSet *write = (VkWriteDescriptorSet *)malloc(sizeof(VkWriteDescriptorSet) * BufferCount);

		for (size_t i = 0; i < BufferCount; i++) {
			write[i] = {};
			write[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write[i].pNext = nullptr;
			write[i].dstSet = set;
			write[i].dstBinding = i;
			write[i].dstArrayElement = 0;
			write[i].descriptorCount = 1;
			write[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write[i].pBufferInfo = &bufferinfo[i];
		}

		vkUpdateDescriptorSets(this->dev.get(), BufferCount, write, 0, NULL);

		free(bufferinfo);
		free(write);
	}

	void Shader::Run(uint32_t x, uint32_t y, uint32_t z)
	{
		VkCommandBufferBeginInfo begininfo = {};
		begininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandbuffer, &begininfo);

		// VK COMMANDS START

		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelinelayout, 0, 1, &set, 0, nullptr);
		vkCmdDispatch(commandbuffer, x, y, z);

		// VK COMMANDS END

		vkEndCommandBuffer(commandbuffer);

		VkSubmitInfo submitinfo = {};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &commandbuffer;

		vkQueueSubmit(dev.getComputeQueue(), 1, &submitinfo, fence);
		vkWaitForFences(dev.get(), 1, &fence, VK_TRUE, UINT64_MAX);
	}

}
