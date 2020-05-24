#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include <vulkan/vulkan.h>
#include "util_exception.h"
#include "vk_device.h"

#include <vector>
#include <string>
#include <cstring>

namespace vkcl {

	class Buffer {
	public:
		Buffer() : released(true), freed(true) { }
		Buffer(const Device &dev, VkDeviceSize size) { Load(dev, size); }
		void Load(const Device &dev, VkDeviceSize size);
		void Load(const Buffer &buf);
		void Delete();
		~Buffer();

		void UploadData(void *data); // Uploads with given size from constructor
		void *GetData(); // Returns address to where data is written
		void ReleaseData(); // Releases objects allocated from GetData();

		inline VkDeviceSize getSize() { return size; }
		inline VkBuffer getHostBuffer() { return HostReadBuffer; }
		inline VkBuffer getDevBuffer() { return DevBuffer; }
	protected:
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageflags, VkMemoryPropertyFlags memflags, VkBuffer &buffer, VkDeviceMemory &buffermemory);
		void CopyBuffer(VkBuffer src, VkBuffer dst);

		Device devcopy;
		VkBuffer HostReadBuffer;
		VkDeviceMemory HostReadBufferMemory;
		VkBuffer DevBuffer;
		VkDeviceMemory DevBufferMemory;
		VkDeviceSize size;

		bool released;
		bool freed;
	};

}

#endif
