#ifndef VK_DEVICE_H
#define VK_DEVICE_H

#include <vkcl/volk.h>

#include "util_exception.h"
#include "util_logging.h"
#include "vk_instance.h"
#include "vk_memory.h"

#include <vector>
#include <string>

namespace vkcl {

	std::vector<VkPhysicalDevice> QueryPhysicalDevices(VkInstance instance);

	struct Buffer {
		VkBuffer devbuffer;
		VmaAllocation devalloc;
		VmaAllocationInfo devinfo;
	};

	struct Shader {
		size_t BufferCount;
		VkCommandBuffer commandbuffer;
		VkShaderModule shadermod;
		VkPipeline pipeline;
		VkPipelineLayout pipelinelayout;
		VkDescriptorPool pool;
		VkDescriptorSet set;
		VkDescriptorSetLayout layout;
		VkFence fence;
	};

	class Device {
	public:
		Device() { }
		Device(const Device &dev);
		Device(VkInstance instance, VkPhysicalDevice PhysicalDevice);
		void Delete();

		void Load(const Device &dev);
		void Load(VkInstance instance, VkPhysicalDevice PhysicalDevice);
		uint32_t MemoryType(uint32_t Type, VkMemoryPropertyFlags Props);

		// Buffer Operations
		Buffer *CreateBuffer(VkDeviceSize size);
		void  DeleteBuffer(Buffer *buffer);
		void  UploadData(Buffer *buffer, void *data);
		void *DownloadData(Buffer *buffer);
		void  ReleaseData(void *data);

		// Compute Operations
		Shader *CreateShader(const std::string fp, size_t BufferCount);
		void DeleteShader(Shader *shader);
		void BindBuffers(Shader *shader, Buffer **buffers);
		void RunShader(Shader *shader, uint32_t x, uint32_t y, uint32_t z);

		inline void setId(uint32_t id) { this->id = id; }
		inline uint32_t getId() { return id; }
		inline std::string getName() { return PhysicalDeviceProps.deviceName; }

		inline VkDevice get() { return device; }
		inline VkQueue getComputeQueue() { return ComputeQueue; }
		inline VkQueue getTransferQueue() { return TransferQueue; }
		inline VkPhysicalDeviceProperties getProps() { return PhysicalDeviceProps; }
		inline VkPhysicalDevice getPhysicalDev() { return PhysicalDevice; }
		inline VkCommandPool getShortCommandPool() { return Pool_ShortLived; } // Command pool for short lived command buffers
		inline VkCommandPool getCommandPool() { return Pool; }
		inline uint32_t *getQueueFamilyIndices() { return QueueFamilyIndices; } // [0] = Compute, [1] = Transfer
		inline VmaAllocator getAllocator() { return allocator; }

		void operator=(const Device &devb);
	protected:
		VkPhysicalDevice PhysicalDevice;
		VkPhysicalDeviceProperties PhysicalDeviceProps;
		VkDevice device;
		VkQueue ComputeQueue;
		VkQueue TransferQueue;
		VkCommandPool Pool_ShortLived;
		VkCommandPool Pool;
		uint32_t QueueFamilyIndices[2];
		uint32_t id;

		VmaAllocator allocator;

		void CreateVKBuffer(VkDeviceSize size, VkBufferUsageFlags usageflags, VkMemoryPropertyFlags memflags, VkBuffer &buffer, VmaAllocation &allocation, VmaAllocationInfo *allocinfo);
		void CopyVKBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

		std::vector<Buffer *> buffers;
	};

	std::vector<vkcl::Device> QueryAllDevices();

}

#endif
